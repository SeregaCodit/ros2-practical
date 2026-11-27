#include <memory>
#include <functional> 
#include <exception>
#include <algorithm>
#include <system_error>
#include <thread>

#include "blink_control/blink_action_server.hpp"
#include "blink_control/action_server_defaults.hpp"


namespace blink_control{

using namespace std::placeholders;


ActionServer::ActionServer(
    const rclcpp::NodeOptions & options
)
: Node("blink_action_server", options)
{

    // topics
    gpio_line_pub_ = create_publisher<std_msgs::msg::Bool>(
        "set_blue_led",
        10
    );

    set_line(false);

    arming_state_sub_ = create_subscription<ArmingState>(
        "arming_state",
        10,
        std::bind(&ActionServer::on_arming_state, this, _1)
    );

    // server
    server_ = rclcpp_action::create_server<BlinkLed>(
        this,
        "blink_led",
        std::bind(&ActionServer::handle_goal, this, _1, _2),
        std::bind(&ActionServer::handle_cancel, this, _1),
        std::bind(&ActionServer::handle_accepted, this, _1)
    );

    RCLCPP_INFO(get_logger(), "INITIALIZED");
}


ActionServer::~ActionServer()
{
    {
        std::lock_guard<std::mutex> lock(state_mutex_);

        stopping_ = true;
        armed_ = false;

        try{
            set_line(false);
        } catch (const std::exception & ex){
            RCLCPP_ERROR(
                get_logger(),
                "Failed to turn off LED:\n%s",
                ex.what()
            );
        }
    }

    wakeup_.notify_all();

    if (worker_.joinable()){
        worker_.join();
    }
}


rclcpp_action::GoalResponse
ActionServer::handle_goal(
    const rclcpp_action::GoalUUID & uuid,
    std::shared_ptr<const BlinkLed::Goal> goal
)
{
    (void)uuid;

    if (
        goal->cycles == 0 ||
        goal->cycles > defaults::MAX_GOAL_CYCLES ||
        goal->half_preiod_ms < defaults::MIN_HALF_PERIOD_MS ||
        goal->half_preiod_ms > defaults::MAX_HALF_PERIOD_MS
    ){
        RCLCPP_ERROR(
            get_logger(),
            "[REJECTED] unconsistent goal parameters!"
        );

        return rclcpp_action::GoalResponse::REJECT;
    }

    std::lock_guard<std::mutex> lock(state_mutex_); 

    if (stopping_ || !armed_){
        RCLCPP_WARN(
            get_logger(),
            "[REJECTED] system is not ready"
        );

        return rclcpp_action::GoalResponse::REJECT;

    }
    
    if (busy_){
        RCLCPP_WARN(
            get_logger(),
            "[REJECTED] another goal is active"
        );

        return rclcpp_action::GoalResponse::REJECT;
    }


    RCLCPP_INFO(
        get_logger(),
        "Goal accepted!"
    );

    busy_ = true;
    disarm_requested_ = false;
    
    RCLCPP_INFO(
        get_logger(),
        "Goal accepted: cycles=%u, half_period=%u ms",
        goal->cycles,
        goal->half_preiod_ms
    );

    return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
}


rclcpp_action::CancelResponse
ActionServer::handle_cancel(
    std::shared_ptr<GoalHandle> goal_handle
)
{
    (void)goal_handle;

    RCLCPP_INFO(
        get_logger(),
        "Cancel request recived"
    );

    // Wake the worker. The worker will check
    // goal_handle->is_canceling().
    wakeup_.notify_all();

    return rclcpp_action::CancelResponse::ACCEPT;
}


void ActionServer::handle_accepted(std::shared_ptr<GoalHandle> goal_handle)
{
    std::thread previous;

    {
        std::lock_guard<std::mutex> lock(state_mutex_);
        previous = std::move(worker_);
    }

    if (previous.joinable()){
        previous.join();
    }

    try{
        {
            std::lock_guard<std::mutex> lock(state_mutex_);

            worker_ = std::thread(
                &ActionServer::execute,
                this,
                goal_handle
            );
        }
    } catch (const std::system_error & ex){
        RCLCPP_ERROR(
            get_logger(),
            "cannot start worker thread:\n%s",
            ex.what()
        );


        // A goal was accepted but execution could
        // not be started. Return ABORTED.
        finish(goal_handle, StopReason::ABORTED, 0);
    }
} 


// EXECUTION
void ActionServer::execute(
    std::shared_ptr<GoalHandle> goal_handle
)
{
    const auto goal = goal_handle->get_goal();
    const auto half_period = 
        std::chrono::milliseconds(goal->half_preiod_ms);

    std::uint32_t completed_cycles = 0;

    StopReason reason = StopReason::NONE;

    RCLCPP_INFO(
        get_logger(),
        "Blink execution started. Goal: %d cycles",
        goal->cycles
    );

    try{
        for (
            std::uint32_t i = 0;
            i < goal->cycles;
            ++i
        ){
            //check if we can continue executing action
            
            reason = interruption_reason(goal_handle);
            
            if (reason != StopReason::NONE){
                break;
            }
            
            // LED ON 
            // must be under mutex if DISARMED will changed
            // already before LED turning on
            {
                std::lock_guard<std::mutex> lock(state_mutex_);

                if (!armed_ || stopping_ || !rclcpp::ok() || disarm_requested_) {
                    reason = StopReason::ABORTED;
                    break;

                } else if (goal_handle->is_canceling()) {
                    reason = StopReason::CANCELED;
                    break;

                } else {
                    set_line(true);
                }
            }

            // wait
           reason = wait_interruptable(goal_handle, half_period);

            if (reason != StopReason::NONE){
                break;
            }
            
            // LED OFF
            // we don`t care about state like turning on case
            set_line(false);

            // wait
            reason = wait_interruptable(goal_handle, half_period);

            if (reason != StopReason::NONE){
                break;
            }

            // feedback
            auto feedback = std::make_shared<BlinkLed::Feedback>();

            feedback->led_on = false;  
            feedback->complited_cycles = ++completed_cycles;
            goal_handle->publish_feedback(feedback);
            
            RCLCPP_DEBUG(
                get_logger(),
                "Completed %d/%d cycles",
                completed_cycles, goal->cycles
            );
        } 
        } catch (const std::exception & ex){
            RCLCPP_ERROR(
                get_logger(),
                "Execution error:\n%s",
                ex.what()
            );

            reason = StopReason::ABORTED;
        }

        finish(goal_handle, reason, completed_cycles);
}


// interruption
ActionServer::StopReason
ActionServer::wait_interruptable(
    const std::shared_ptr<GoalHandle> & goal_handle,
    const std::chrono::milliseconds period
){
    const auto deadline = std::chrono::steady_clock::now() + period;
    const auto wakeup_period = 
        std::chrono::milliseconds(defaults::WAKEUP_PERIOD_MS); 

    while (true){ 
        const auto reason = interruption_reason(goal_handle);
        if (reason != StopReason::NONE){
            return reason;
        } 

        auto now = std::chrono::steady_clock::now();
        if (now >= deadline){
            break;
        }

        auto next_wakeup = std::min(deadline, now + wakeup_period);
        {
            std::unique_lock<std::mutex> lock(state_mutex_);
            wakeup_.wait_until(lock, next_wakeup);
        }


    }

    return interruption_reason(goal_handle);
}


ActionServer::StopReason 
ActionServer::interruption_reason(const std::shared_ptr<GoalHandle> & goal_handle){
     if (goal_handle->is_canceling()){
        return StopReason::CANCELED;
    }

    {   
        std::lock_guard<std::mutex> lock(state_mutex_);

        if (!armed_ ||
            stopping_ ||
            disarm_requested_
        ){
            return StopReason::ABORTED;
        }
    }

    if(!rclcpp::ok()){
        return StopReason::ABORTED;
    }

    return StopReason::NONE;
}

void ActionServer::finish(
    std::shared_ptr<GoalHandle> goal_handle,
    StopReason reason,
    std::uint32_t completed_cycles
){

    // firstable set line INACTIVE
    try{
        set_line(false);
    } catch (std::exception & ex) {

        RCLCPP_ERROR(
            get_logger(),
            "Cannot publish LED OFF: %s",
            ex.what()
        );

        reason = StopReason::ABORTED;

    }


    auto final_reason = interruption_reason(goal_handle);
    
    if (final_reason != StopReason::NONE){
        reason = final_reason;
    }

    // PREPEARE RESULT
    auto result = std::make_shared<BlinkLed::Result>();
    result->complited_cycles = completed_cycles;
    
    bool finalized = false;

    try{

        if (goal_handle->is_canceling()){
            goal_handle->canceled(result);

            RCLCPP_INFO(
                get_logger(),
                "Goal CAMCELED. %d cycles completed",
                completed_cycles
            );
        } else if (reason == StopReason::NONE) {
            goal_handle->succeed(result);

            RCLCPP_INFO(
                get_logger(),
                "Goal SUCCESSED. %d completed cycles",
                completed_cycles
            );
        } else {
            goal_handle->abort(result);

            RCLCPP_WARN(
                get_logger(),
                "Goal ABORTED. %d completed cycles",
                completed_cycles
            );
        }

        finalized = true;
    } catch (std::exception & ex){
        RCLCPP_ERROR(
            get_logger(),
            "FINALIZATION ERROR: %s",
            ex.what()
        );
    }


    // Release execution slot.
    {
        std::lock_guard<std::mutex> lock(state_mutex_);

        if(
            finalized ||
            stopping_ ||
            !rclcpp::ok()
        ){
            busy_ = false;
        }
    }
    
    
}


// LINE CONTROL
void ActionServer::set_line(const bool active){
    std_msgs::msg::Bool msg;
    msg.data = active;
    gpio_line_pub_->publish(msg);
}

// sub
void ActionServer::on_arming_state(const ArmingState & msg){
    const bool new_state = msg.state == ArmingState::ARMED;

    std::lock_guard<std::mutex> lock(state_mutex_);
    
    if (armed_ && !new_state){
        
        if (busy_){
            disarm_requested_ = true;
        }

        
        RCLCPP_INFO(
            get_logger(),
            "System DISARMED!"
        );
        
        set_line(false);
        wakeup_.notify_all();
        
    }
    
    armed_ = new_state;
}

} // namespace blink_control