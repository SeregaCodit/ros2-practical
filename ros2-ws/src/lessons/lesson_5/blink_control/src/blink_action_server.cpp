#include <memory>
#include <functional> 
#include <exception>

#include "blink_control/blink_action_server.hpp"
#include "blink_control/action_server_params.hpp"


namespace blink_control{

using namespace std::placeholders;
using namespace std::chrono_literals;

ActionServer::ActionServer(
    const rclcpp::NodeOptions & options
)
: Node("blink_action_server", options)
{
    // params
    declare_parameter(keys::GPIO_LINE, defaults::GPIO_LINE);

    // topics
    gpio_line_pub_ = create_publisher<std_msgs::msg::Bool>(
        "set_gpio_line",
        10
    );

    set_line(false);

    arming_state_sub_ = create_subscription<ArmingState>(
        "arming_atste",
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
        ++disarm_epoch_;

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

    std::lock_guard<std::mutex> lock(state_mutex_);  // a k threadig.lock() ?

    
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

    std::lock_guard<std::mutex> lock(state_mutex_);

    try{
        worker_ = std::thread(
            &ActionServer::execute,
            this,
            goal_handle
        );
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


void ActionServer::finish(
    std::shared_ptr<GoalHandle> goal_handle,
    StopReason reason,
    std::uint32_t completed_cycles
){

    // firstable set lune INACTIVE
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
}

// INTERRUPTION


// LINE CONTROL
void ActionServer::set_line(const bool active){
    std_msgs::msg::Bool msg;
    msg.data = active;
    gpio_line_pub_->publish(msg);
}

// sub
void ActionServer::on_arming_state(const ArmingState msg){
    armed_ = msg.state;

    if (!armed_){

        set_line(false);

        // Wake the worker to recheck the ARM state.
        wakeup_.notify_all();

        RCLCPP_INFO(get_logger(), "System DISARMED - Goal ABORTED!");
    }
}

} // namespace blink_control