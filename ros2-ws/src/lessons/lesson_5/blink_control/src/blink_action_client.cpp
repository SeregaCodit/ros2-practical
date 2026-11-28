#include "blink_control/blink_action_client.hpp"


namespace blink_control {

using namespace std::placeholders;

ActionClient::ActionClient(
    const rclcpp::NodeOptions & options
)
: Node("blink_action_client", options)
{
    // params
    param_listener_ =
    std::make_shared<action_client_parameters::ParamListener>(
        get_node_parameters_interface()
    );

    // interfaces
    event_sub_ = create_subscription<std_msgs::msg::Bool>(
        "trigger_topic",
        10,
        std::bind(&ActionClient::on_short_single_press, this, _1)
    );

    client_ = rclcpp_action::create_client<BlinkLed>(
        this,
        "blink_led"
    );

    RCLCPP_INFO(
        get_logger(),
        "INITIALIZED"
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
        if (!client_->action_server_is_ready()){
            RCLCPP_ERROR(
                get_logger(),
                "Blink Action Server unavailable"
            );
            return;
        }

        if (send_goal()){
            state_ = State::PENDING;
        }

        break;
    case State::PENDING:
        state_ = State::CANCEL_REQUESTED;
        break;
    case State::ACTIVE:
        cancel_request();
        break;
    default:
        break;
    }
}


bool ActionClient::send_goal(){
    // get params
    const auto params = param_listener_->get_params();

    auto cycles = params.cycles;
    auto half_period_ms = params.half_period_ms;
    
    // goal
    auto goal_msg = BlinkLed::Goal();
    goal_msg.cycles = std::int32_t(cycles);
    goal_msg.half_preiod_ms = std::int32_t(half_period_ms);

    RCLCPP_INFO(
        get_logger(),
        "Sending goal: cycles = %ld, time_period_ms = %ld",
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

    return true;
}


void ActionClient::goal_response_callback(
    const GoalHandle::SharedPtr & goal_handle
){
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
            
            RCLCPP_INFO(
                get_logger(),
                "SUCCEEDED %d cycles completed",
                result.result->complited_cycles
            );

            break;

        case rclcpp_action::ResultCode::CANCELED:

            RCLCPP_INFO(
                get_logger(),
                "CANCELED %d cycles completed",
                result.result->complited_cycles
            );
            break;

        case rclcpp_action::ResultCode::ABORTED:
            
            RCLCPP_WARN(
                get_logger(),
                "ABORTED. %d cycles completed",
                result.result->complited_cycles
            );
            
            break;

        default:
            RCLCPP_ERROR(
                get_logger(),
                "UNKNOWN complete code. %d cycles completed",
                result.result->complited_cycles
            );
            break;
    }

    current_goal_.reset();
    state_ = State::IDLE;
}

} // namespace blink_control