#pragma once

#include <memory>
#include <future>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"

#include "std_msgs/msg/bool.hpp"
#include "system_interfaces/action/blink_led.hpp"

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

    // TYPES
    using BlinkLed = system_interfaces::action::BlinkLed;
    using GoalHandle = rclcpp_action::ClientGoalHandle<BlinkLed>;
    
    // ROS2 interfaces
    rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr event_sub_;
    rclcpp_action::Client<BlinkLed>::SharedPtr client_;

    // callbacks
    void goal_response_callback(
        std::shared_future<GoalHandle::SharedPtr> future
    );

    void feedback_calback(
        GoalHandle::SharedPtr,
        const std::shared_ptr<const BlinkLed::Feedback> feedback
    );

    void result_callback(
        GoalHandle::WrappedResult & result
    );

    void send_goal();

    // internal states
    State state_{State::IDLE};

}; // class ActionClient

} // namespace blink_control