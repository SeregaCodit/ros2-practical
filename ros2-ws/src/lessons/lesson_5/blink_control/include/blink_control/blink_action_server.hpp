#pragma once

#include <chrono>
#include <memory>

#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/rclcpp_action.hpp>

#include "std_msgs/msg/bool.hpp"

#include "system_interfaces/msg/arming_state.hpp"
#include "system_interfaces/action/blink_led.hpp"


namespace blink_control{

class ActionServer final : public rclcpp::Node
{
public:
    ActionServer();

private:
    // STATE MACHINES
    enum class FinishReason{
        SUCCESSED,
        CANCELED,
        ABORTED
    };

    // TYPES
    using BlinkLed = system_interfaces::action::BlinkLed;
    using ArmingState = system_interfaces::msg::ArmingState;
    using GoalHandle = rclcpp_action::ServerGoalHandle<BlinkLed>;

    // ROS2 INTERFACES
    rclcpp::Subscription<ArmingState>::SharedPtr arming_state_sub_;
    rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr gpio_line_pub_;

    rclcpp_action::Server<BlinkLed>::SharedPtr server_;

    rclcpp::TimerBase::SharedPtr timer_;

    // internal state
    std::shared_ptr<GoalHandle> active_goal_;

    bool armed_{false};
    bool gaol_reserved_{false};
    bool line_active_{false};
    uint32_t completed_cycles_{0};

    std::chrono::steady_clock::time_point next_toggle_;

    // callbacks
    rclcpp_action::GoalResponse handle_goal(
        const rclcpp_action::GoalUUID & uuid,
        std::shared_ptr<const BlinkLed::Goal> goal
    );

    rclcpp_action::CancelResponse handle_cancel(
        std::shared_ptr<GoalHandle> goal_handle
    );

    void handle_accepted(
        std::shared_ptr<GoalHandle> goal_handle
    );

    void handle_arming_state(
        ArmingState::SharedPtr msg
    );

    void tick();
    void set_line();
    void finish(FinishReason reason);
};

} // blink_control