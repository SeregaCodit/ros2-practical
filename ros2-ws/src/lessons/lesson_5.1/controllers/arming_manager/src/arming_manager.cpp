#include <rclcpp/rclcpp.hpp>

#include <sensor_msgs/msg/battery_state.hpp>
#include <sensor_msgs/msg/range.hpp>

#include <chrono>
#include <string>
#include <iostream>
#include <stdexcept>
#include <cstdint>
#include <cmath>

#include "safety_interfaces/msg/arm_state.hpp"
#include "safety_interfaces/msg/arm_decision.hpp"
#include "safety_interfaces/srv/set_arm.hpp"

#include "arming_manager/params.hpp"

using SetArm = safety_interfaces::srv::SetArm;
using ArmDecision = safety_interfaces::msg::ArmDecision;
using ArmState = safety_interfaces::msg::ArmState;

using BatteryState = sensor_msgs::msg::BatteryState;
using Range = sensor_msgs::msg::Range;
using Clock = std::chrono::steady_clock;
using Duration = std::chrono::milliseconds;


class ArmingManager final: public rclcpp::Node
{
public:
    ArmingManager()
    : Node ("arming_manager")
    {   

        // ArmState init
        current_arm_state_.stamp = now();
        current_arm_state_.state = ArmState::STANDBY;
        current_arm_state_.revision = 0;

        // sensors heartbeat
        auto timeout = declare_parameter<int64_t>("heartbeat_timeout_ms", 500);

        if (timeout < params::MIN_HEARTBEAT_TIMEOUT){
            RCLCPP_WARN(get_logger(),
                "heartbeat_timeout_ms must be >= %d ms\nfallback to %d ms",
                params::MIN_HEARTBEAT_TIMEOUT);
            
            timeout = params::MIN_HEARTBEAT_TIMEOUT;
        };

        heartbeat_timeout_ = Duration{timeout};

        //battery
        min_arm_voltage_ = declare_parameter<double>("min_arm_voltage_v", 7.2);



        battery_state_sub_ = create_subscription<BatteryState>(
            "battery_state_topic", // define real name with remapping
            rclcpp::QoS(10),
            [this](const BatteryState &msg){

                battery_state_msg_ = msg;
                has_battery_state_ = true;
            }
        );

        // Range
        min_obstacle_dist_m_ = declare_parameter<double>("min_obstacle_dist_m", 0.4);

        range_sensor_sub_ = create_subscription<Range>(
            "range_sensor_topic",
            rclcpp::QoS(10),
            [this](const Range &msg){
                range_sensor_msg_ = msg;
                has_range_state_ = true;
            }
        );

        service_ = create_service<SetArm>(
            "arm_manager",
            [this](const std::shared_ptr<SetArm::Request> request,
                std::shared_ptr<SetArm::Response> response)
                {
                    handle_arm_request(request, response);
                }
        );

        
    };

private:

    bool check_time_freshness(const std_msgs::msg::Header &header){
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

    
    int battery_check(){
        const auto &msg = battery_state_msg_;
        if (!has_battery_state_){
            return ArmDecision::UPS_UNAVAILABLE;
        }

        if (!check_time_freshness(msg.header)){
            return ArmDecision::UPS_UNAVAILABLE;
        }
        
        if (!std::isfinite(msg.voltage) ||
            msg.voltage < 0.0f){
            return ArmDecision::UPS_INVALID;
        }

        if (min_arm_voltage_ > msg.voltage){
            return ArmDecision::LOW_VOLTAGE;
        }

        return ArmDecision::OK;
    }


    int range_check(){
        const auto &msg = range_sensor_msg_;
        if (!has_range_state_){
            return ArmDecision::RANGE_SENSOR_UNAVAILABLE;
        }

        if (!check_time_freshness(msg.header)){
            return ArmDecision::RANGE_SENSOR_UNAVAILABLE;
        }

        if (!std::isfinite(msg.range) ||
            !std::isfinite(msg.max_range) ||
            !std::isfinite(msg.min_range) ||
            msg.min_range < 0.0f ||
            msg.min_range >= msg.max_range ||
            msg.range < msg.min_range || 
            msg.range > msg.max_range){
            return ArmDecision::RANGE_SENSOR_INVALID;
        }

        if (min_obstacle_dist_m_ > msg.range){
            return ArmDecision::OBSTACLE_TOO_CLOSE;
        }

        return ArmDecision::OK;
    }


    void update_current_arm_state(const int &target_state){

        if (current_arm_state_.state != target_state){
            current_arm_state_.state = target_state;
            current_arm_state_.stamp = now();
            current_arm_state_.revision ++;   
        }
    }


    void handle_arm_request(const std::shared_ptr<SetArm::Request> request,
        std::shared_ptr<SetArm::Response> response){
            auto &descision = response->decision;
            
            descision.command = request->command;
            descision.accepted = false;

            // command validation
            if (request->command != SetArm::Request::ARM &&
                request->command != SetArm::Request::DISARM){
                    descision.reason = ArmDecision::INVALID_COMMAND;
                    descision.detail = "invalid command " + 
                        std::to_string(request->command);
                    
                    response->state = current_arm_state_;
                        return;
                }

            // DISARM IS UNCONDITIONAL
            if (request->command == SetArm::Request::DISARM){
                descision.accepted = true;
                descision.reason = ArmDecision::OK;
                descision.detail = "DISARM accepted";

                update_current_arm_state(ArmState::STANDBY);

                response->state = current_arm_state_;
                return;
            }

            // sensors check
            auto bat_reason = battery_check();
            if (bat_reason != ArmDecision::OK){
                descision.reason = bat_reason;
                descision.detail = "battery state is unsafe";

                response->state = current_arm_state_;
                return;
            }

            auto range_reason = range_check();
            if (range_reason != ArmDecision::OK){
                descision.reason = range_reason;
                descision.detail = "unconsistent range data";

                response->state = current_arm_state_;
                return;
            }

            // all checks ok
            descision.accepted = true;
            descision.reason = ArmDecision::OK;
            descision.detail = "ARM accepted";
            update_current_arm_state(ArmState::ARMED);
            response->state = current_arm_state_;


    };

    BatteryState battery_state_msg_;
    Range range_sensor_msg_;
    ArmState current_arm_state_{};

    bool has_battery_state_{false};
    bool has_range_state_{false};

    double min_arm_voltage_;
    double min_obstacle_dist_m_;

    Duration heartbeat_timeout_;

    rclcpp::Service<SetArm>::SharedPtr service_;
    rclcpp::Subscription<BatteryState>::SharedPtr battery_state_sub_;
    rclcpp::Subscription<Range>::SharedPtr range_sensor_sub_;
};

