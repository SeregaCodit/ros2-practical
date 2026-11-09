#include <rclcpp/rclcpp.hpp>

#include <sensor_msgs/msg/battery_state.hpp>

#include <chrono>
#include <string>
#include <iostream>
#include <stdexcept>
#include <cstdint>

#include "safety_interfaces/msg/arm_state.hpp"
#include "safety_interfaces/msg/arm_decision.hpp"
#include "safety_interfaces/srv/set_arm.hpp"

#include "arming_manager/params.hpp"

using SetArm = safety_interfaces::srv::SetArm;
using BatteryState = sensor_msgs::msg::BatteryState;
using Clock = std::chrono::steady_clock;
using Duration = std::chrono::milliseconds;


class ArmingManager final: public rclcpp::Node
{
public:
    ArmingManager()
    : Node ("arming_manager")
    {   
        const auto timeout = declare_parameter<int64_t>("heartbeat_timeout_ms", 500);

        if (timeout <= params::MIN_HEARTBEAT_TIMEOUT){
            throw std::invalid_argument("heartbeat timeout must be >=" +
                std::to_string(params::MIN_HEARTBEAT_TIMEOUT));
        };

        heartbeat_timeout_ = Duration{timeout};

        service_ = create_service<SetArm>(
            "arm_manager",
            [this](const std::shared_ptr<SetArm::Request> request,
                std::shared_ptr<SetArm::Response> response)
                {
                    handle_arm_request(request, response);
                }
        );

        battery_state_sub_ = create_subscription<BatteryState>(
            "battery_state", // define real name with remapping
            rclcpp::QoS(10),
            [this](const BatteryState &msg){

                battery_state_msg_ = msg;
                has_battery_state_ = true;
            }
        );
    };

private:
    bool check_time_freshness(const std_msgs::msg::Header header){
        // check if msg time is fresh
        const rclcpp::Time stamp{
            header.stamp,
            get_clock()->get_clock_type()
        };

        const auto age = now() - stamp;  // stamp in nanoseconds 
        const rclcpp::Duration timeout(heartbeat_timeout_);
        return age >= rclcpp::Duration::from_seconds(0)
            && age <= timeout;
    }

    bool battery_check(){
        if (!has_battery_state_){
            return false;
        }

        if (!check_time_freshness(battery_state_msg_.header)){
            return false;
        }

    }

    void handle_arm_request(const std::shared_ptr<SetArm::Request> request,
        std::shared_ptr<SetArm::Response> response){

            // battery check
        

    };
    BatteryState battery_state_msg_;
    bool has_battery_state_;

    Duration heartbeat_timeout_;

    rclcpp::Service<SetArm>::SharedPtr service_;
    rclcpp::Subscription<BatteryState>::SharedPtr battery_state_sub_;
};

