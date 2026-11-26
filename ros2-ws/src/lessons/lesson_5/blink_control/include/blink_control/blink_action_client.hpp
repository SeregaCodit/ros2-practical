#pragma once

#include <memory>
#include <future>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"

#include "std_msgs/msg/bool.hpp"
#include "system_interfaces/action/blink_led.hpp"

#include "blink_control/action_client_params.hpp"

namespace blink_control{

class ActionClient final : public rclcpp::Node
{
public:
    explicit ActionClient(
        const rclcpp::NodeOptions & options =
            rclcpp::NodeOptions()
    );

private:
    // STATES
    enum class State{
        IDLE,
        PENDING,
        ACTIVE,
        CANCEL_REQUESTED
    };

    // PARAMS
    std::shared_ptr<action_client_parameters::ParamListener> param_listener_;

    // TYPES
    using BlinkLed = system_interfaces::action::BlinkLed;
    using GoalHandle = rclcpp_action::ClientGoalHandle<BlinkLed>;
    
    // ROS2 interfaces
    rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr event_sub_;
    rclcpp_action::Client<BlinkLed>::SharedPtr client_;

    // client callbacks
    void goal_response_callback(
        const GoalHandle::SharedPtr & goal_handle
    );

    void feedback_callback(
        GoalHandle::SharedPtr goal_handle,
        const std::shared_ptr<const BlinkLed::Feedback> feedback
    );

    void result_callback(
        const GoalHandle::WrappedResult & result
    );

    // topic callbacks
    void on_short_single_press(const std_msgs::msg::Bool & msg);

    // methods
    bool send_goal();
    void cancel_request();

    // internal states
    State state_{State::IDLE};
    GoalHandle::SharedPtr current_goal_;

}; // class ActionClient

} // namespace blink_control