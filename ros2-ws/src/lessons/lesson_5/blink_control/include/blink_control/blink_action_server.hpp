#pragma once

#include <chrono>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <cstdint>
#include <thread>

#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/rclcpp_action.hpp>

#include "std_msgs/msg/bool.hpp"
#include "system_interfaces/msg/arming_state.hpp"
#include "system_interfaces/action/blink_led.hpp"


namespace blink_control{

class ActionServer final : public rclcpp::Node
{
public:
    explicit ActionServer(
        const rclcpp::NodeOptions & options = 
            rclcpp::NodeOptions()
    );

    ~ActionServer() override;


private:
    // STATE MACHINES
    enum class StopReason{
        NONE,
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

    // helping fucntions
    void finish(
        std::shared_ptr<GoalHandle> goal_handle,
        StopReason reason,
        std::uint32_t completed_cycles
    );

    void execute(
        std::shared_ptr<GoalHandle> goal_handle
    );

    void on_arming_state(const ArmingState & msg);

    // interruption
    StopReason interruption_reason(const std::shared_ptr<GoalHandle> & goal_handle);
    
    StopReason wait_interruptable(
        const std::shared_ptr<GoalHandle> & goal_handle,
        const std::chrono::milliseconds period
    );

    // LED control
    void set_line(const bool active);

    // thread sychronisation
    mutable std::mutex state_mutex_;
    std::condition_variable wakeup_;
    std::thread worker_;
    

    // internal state
    bool armed_{false};
    bool busy_{false};
    bool stopping_{false};
    bool disarm_requested_{false};  
};

} // blink_control