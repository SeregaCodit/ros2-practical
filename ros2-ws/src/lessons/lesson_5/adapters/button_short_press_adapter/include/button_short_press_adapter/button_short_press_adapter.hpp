#pragma once

#include <chrono>
#include <rclcpp/rclcpp.hpp>
#include "std_msgs/msg/bool.hpp"


class ButtonShortPressAdapter final : public rclcpp::Node
{
public:
    ButtonShortPressAdapter();

private:
    // types
    using Clock = std::chrono::steady_clock;
    
    // callbacks
    void on_button_state(
        const std_msgs::msg::Bool::SharedPtr msg
    );

    // ROS2 interfaces
    rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr button_sub_;
    rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr short_press_publisher_;

    // inernal state
    Clock::time_point button_pressed_at_{};

    bool prev_state_{false};
    bool triggered_{false};
};