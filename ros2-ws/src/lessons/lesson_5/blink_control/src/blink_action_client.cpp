#include "blink_control/blink_action_client.hpp"
#include "blink_control/action_client_params.hpp"


namespace blink_control {

using namespace std::placeholders;

// constructor
ActionClient::ActionClient(
    const rclcpp::NodeOptions & options
)
: Node("blink_action_client", options)
{
    // params
    declare_parameter(keys::HALF_PERIOD_MS, defaults::HALF_PERIOD_MS);
    declare_parameter(keys::CYCLES, defaults::CYCLES);

    // interfaces
    event_sub_ = create_subscription<std_msgs::msg::Bool>(
        "button/short_single_press",
        10,
        std::bind(&ActionClient::on_short_single_press, this, _1)
    );

    client_ = rclcpp_action::create_client<BlinkLed>(
        this,
        "blink_led"
    );
}


void ActionClient::on_short_single_press(
    const std_msgs::msg::Bool & msg
){

    if (!msg.data){
        return;
    }

    switch (state_)
    {
    case State::IDLE:
        if (!client_->wait_for_action_server()){
            RCLCPP_ERROR(
                get_logger(),
                "Blink Action Server not available after waiting!"
            );
            return;
        }

        send_goal();
        state_ = State::PENDING;
        break;
    case State::PENDING:
        cancel_request();
        state_ = State::CANCEL_REQUESTED;
        break;
    case State::ACTIVE:
        cancel_request();
        state_ = State::CANCEL_REQUESTED;
        break;
    default:
        break;
    }
}


void ActionClient::send_goal(){
    // get and validate params

    auto cycles = get_parameter(keys::CYCLES).as_int();
    
    if (cycles <= 0)
    {
        RCLCPP_ERROR(
            get_logger(),
            "Cycles parameter must be greater than 0. Got %d",
            cycles
        );
        
        return;
    }
    
    auto half_period_ms = get_parameter(keys::HALF_PERIOD_MS).as_int();

    if (half_period_ms < defaults::MIN_HALF_PERIOD_MS)
    {
        RCLCPP_ERROR(
            get_logger(),
            "half_period_ms parameter must be greater than %d. Got %d",
            defaults::MIN_HALF_PERIOD_MS, half_period_ms
        );
        
        return;
    }
    
    // goal
    auto goal_msg = BlinkLed::Goal();
    goal_msg.cycles = cycles;
    goal_msg.half_preiod_ms = half_period_ms;

    RCLCPP_INFO(
        get_logger(),
        "Sending goal: cycles = %d, time_period_ms = %d",
        cycles, half_period_ms
    );

    auto send_goal_options = rclcpp_action::Client<BlinkLed>::SendGoalOptions();

    send_goal_options.goal_response_callback =
        std::bind(&ActionClient::goal_response_callback, this, _1);
    send_goal_options.feedback_callback =
        std::bind(&ActionClient::feedback_callback, this, _1, _2);
    send_goal_options.result_callback =
        std::bind(&ActionClient::result_callback, this, _1);

    client_->async_send_goal(goal_msg, send_goal_options);
}


void ActionClient::goal_response_callback(
    const GoalHandle::SharedPtr & goal_handle
){
    // цей калбек спрацьовує лише один раз при надсиланні цілі? 
    if (!goal_handle){
        RCLCPP_ERROR(
            get_logger(),
            "Goal was rejected by server"
        );

        state_ = State::IDLE;
        current_goal_.reset();
        return;
    }

    current_goal_ = goal_handle;

    const bool cancel_now = (state_ == State::CANCEL_REQUESTED);

    if (!cancel_now){
        state_ = State::ACTIVE;
    } else {
        cancel_request();
    }

}


void ActionClient::cancel_request(){
    try{
        client_->async_cancel_goal(current_goal_);
        state_ = State::CANCEL_REQUESTED;
        
        RCLCPP_INFO(
            get_logger(),
            "Goal cangel requested"
        );

    } catch (std::exception & ex) {
        
        RCLCPP_ERROR(
            get_logger(),
            "Goal cangeling failed: %s", ex.what()
        );
    }
}


void ActionClient::feedback_callback(
    GoalHandle::SharedPtr goal_handle,
    const std::shared_ptr<const BlinkLed::Feedback> feedback
){
    (void)goal_handle;

    RCLCPP_INFO(
        get_logger(),
        "FEEDBACK: %d cycles completed",
        feedback->complited_cycles
    );
}


void ActionClient::result_callback(
    const GoalHandle::WrappedResult & result
){
    switch (result.code)
    {
    case rclcpp_action::ResultCode::SUCCEEDED:
        state_ = State::IDLE;

        RCLCPP_INFO(
            get_logger(),
            "SUCCEEDED Goal completed, ready to new goal"
        );
        break;
    
    // TODO
    
    default:
        break;
    }
}

} // namespace blink_control