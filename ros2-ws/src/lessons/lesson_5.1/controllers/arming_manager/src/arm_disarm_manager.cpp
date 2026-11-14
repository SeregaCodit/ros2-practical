#include <rclcpp/rclcpp.hpp>

#include <sensor_msgs/msg/battery_state.hpp>
#include <sensor_msgs/msg/range.hpp>

#include <chrono>
#include <string>
#include <iostream>
#include <stdexcept>
#include <cstdint>
#include <cmath>


#include "safety_interfaces/msg/arm_disarm_decision.hpp"
#include "safety_interfaces/srv/accept_arm_disarm.hpp"
#include "system_interfaces/msg/arming_state.hpp"

#include "arming_manager/params.hpp"


using ArmDisarmDecision = safety_interfaces::msg::ArmDisarmDecision;
using ArmingState = system_interfaces::msg::ArmingState;
using AcceptArmDisarm = safety_interfaces::srv::AcceptArmDisarm;

using BatteryState = sensor_msgs::msg::BatteryState;
using Range = sensor_msgs::msg::Range;
using Clock = std::chrono::steady_clock;
using Duration = std::chrono::milliseconds;

using namespace std::chrono_literals;



class ArmingManager final: public rclcpp::Node
{
public:
    ArmingManager()
    : Node ("arm_disarm_manager")
    {   

        // sensors heartbeat
        declare_parameter<int64_t>(keys::HEARTBEAT_TIMEOUT_MS,
             defaults::MIN_HEARTBEAT_TIMEOUT);


        //battery
        declare_parameter<double>(keys::MIN_ARM_VOLTAGE_V,
             defaults::MIN_ARM_VOLTAGE);

        battery_state_sub_ = create_subscription<BatteryState>(
            "battery_state", // define real name with remapping
            rclcpp::QoS(10),
            [this](const BatteryState &msg){

                battery_state_msg_ = msg;
                has_battery_state_ = true;
            }
        );
        
        // Range
        declare_parameter<double>(keys::MIN_OBSTACLE_DIST_M,
            defaults::MIN_OBSTACLE_DIST_M);

        range_sensor_sub_ = create_subscription<Range>(
            "range_sensor_topic",
            rclcpp::QoS(10),
            [this](const Range &msg){
                range_sensor_msg_ = msg;
                has_range_state_ = true;
            }
        );
        // Service
        service_ = create_service<AcceptArmDisarm>(
            "arm_manager",
            [this](const std::shared_ptr<AcceptArmDisarm::Request> request,
                std::shared_ptr<AcceptArmDisarm::Response> response)
                {
                    handle_arm_request(request, response);
                }
        );
        // // ArmingState publishing
        // arm_state_publisher_ = create_publisher<ArmingState>(
        //     "arm_state",
        //     rclcpp::QoS(10)
        // );

        // arm_state_pub_timer_ = create_wall_timer(
        //     100ms,
        //     [this](){
        //         arm_state_publisher_->publish(current_arm_state_);
        //     }
        // ); 

    //     // ArmDisarmDecision publishing
    //     arm_decision_publisher_ = create_publisher<ArmDisarmDecision>(
    //         "arm_decision",
    //         rclcpp::QoS(10)
    //     );

        RCLCPP_INFO(get_logger(), "INITIALIZED");
    };


private:
    bool check_time_freshness(const std_msgs::msg::Header &header){
        /*  
            check if msg time is fresh enought
            it'enought for now but we can also use msg reciving time 
            for usin in complimentary filters or KF / EKF filters 
        */
        const rclcpp::Time stamp{
            header.stamp,
            get_clock()->get_clock_type()
        };

        const auto age = now() - stamp;  // stamp in nanoseconds 

        // cast from int64_t to std::chrono::milliseconds
        auto hb_timeout = Duration{get_parameter(keys::HEARTBEAT_TIMEOUT_MS).as_int()};
        
        // than cast to rclcpp::Duration
        const rclcpp::Duration timeout(hb_timeout);

        if (age >= rclcpp::Duration::from_seconds(0)
            && age <= timeout){
                RCLCPP_INFO(get_logger(), "[OK] - msg age check");
                return true;
            }
        
        RCLCPP_ERROR(get_logger(),
            "Sensor timestamp invalid: age=%.3f s, timeout=%.3f s",
            age.seconds(), timeout.seconds()
        );
        return false;
    }

    
    int battery_check(){
        const auto &msg = battery_state_msg_;
        if (!has_battery_state_){
            RCLCPP_ERROR(get_logger(), "No battery_state!");
            return ArmDisarmDecision::UPS_UNAVAILABLE;
        }

        if (!check_time_freshness(msg.header)){
            RCLCPP_ERROR(get_logger(),
                "battery state msg is too old"
            );
            return ArmDisarmDecision::UPS_UNAVAILABLE;
        }
        
        if (!std::isfinite(msg.voltage) ||
            msg.voltage < 0.0f){
                RCLCPP_ERROR(get_logger(),
                "battery voltage must be > 0.0. Got %f",
                msg.voltage
            );
            return ArmDisarmDecision::UPS_INVALID;
        }

        auto min_arm_voltage = get_parameter(keys::MIN_ARM_VOLTAGE_V).as_double();
        if (min_arm_voltage > msg.voltage){
            RCLCPP_ERROR(get_logger(),
                "battery voltage is lower safe thrashold. Got %f, threshold %f",
                msg.voltage, defaults::MIN_ARM_VOLTAGE
            );
            return ArmDisarmDecision::LOW_VOLTAGE;
        }

        RCLCPP_INFO(get_logger(), "[OK] - battery check");
        return ArmDisarmDecision::OK;
    }


    int range_check(){
        const auto &msg = range_sensor_msg_;
        if (!has_range_state_){
            RCLCPP_ERROR(get_logger(),
                "no range sensor data"
            );
            return ArmDisarmDecision::RANGE_SENSOR_UNAVAILABLE;
        }

        if (!check_time_freshness(msg.header)){
            RCLCPP_ERROR(get_logger(),
                "range sensor msg is too old"
            );
            return ArmDisarmDecision::RANGE_SENSOR_UNAVAILABLE;
        }

        // accept +inf values for range sensor
        if (std::isnan(msg.range) ||  
            std::isnan(msg.max_range) ||
            std::isnan(msg.min_range) ||
            msg.min_range < 0.0f ||
            // reject msg.min_range == msg.max_range case
            // (for bool range sensors)
            msg.min_range >= msg.max_range ||
            msg.range < msg.min_range
            // accept +inf values for range sensor
            // msg.range > msg.max_range
            ){
                RCLCPP_ERROR_STREAM(get_logger(),
                "range data is unconsistent: \n" 
                << sensor_msgs::msg::to_yaml(msg)
            );
            return ArmDisarmDecision::RANGE_SENSOR_INVALID;
        }

        auto min_dist = get_parameter(keys::MIN_OBSTACLE_DIST_M).as_double();
        if (min_dist > msg.range){
            RCLCPP_ERROR(get_logger(), 
                "obstacle is too close (%f) m. Min: %f",
                msg.range, min_dist
            );
            return ArmDisarmDecision::OBSTACLE_TOO_CLOSE;
        }

        RCLCPP_INFO(get_logger(), "[OK] range check");
        return ArmDisarmDecision::OK;
    }


    // void update_current_arm_state(const uint8_t target_state){
        
    //     if (current_arm_state_.state != target_state){
    //         current_arm_state_.state = target_state;
    //         current_arm_state_.stamp = now();
    //         current_arm_state_.revision ++;   
    //     }

    //     RCLCPP_INFO_STREAM(get_logger(),
    //         "arm state updated: \n"
    //         << safety_interfaces::msg::to_yaml(current_arm_state_)
    //     );
    // }


    void handle_arm_request(const std::shared_ptr<AcceptArmDisarm::Request> request,
        std::shared_ptr<AcceptArmDisarm::Response> response){
            auto &decision = response->decision;
            
            decision.command = request->command;
            decision.accepted = false;

            // command validation
            if (request->command != AcceptArmDisarm::Request::ARM &&
                request->command != AcceptArmDisarm::Request::DISARM){
                    decision.reason = ArmDisarmDecision::INVALID_COMMAND;
                    decision.detail = "invalid command " + 
                        std::to_string(request->command);
                    
                    current_arm_decision_ = decision;

                    arm_decision_publisher_->publish(current_arm_decision_);
                    return;
                }

            // DISARM IS UNCONDITIONAL
            if (request->command == AcceptArmDisarm::Request::DISARM){
                decision.accepted = true;
                decision.reason = ArmDisarmDecision::OK;
                decision.detail = "DISARM accepted";
                current_arm_decision_ = decision;

                arm_decision_publisher_->publish(current_arm_decision_);
                return;
            }

            // if (current_arm_state_.state == ArmingState::ARMED){
            //     decision.accepted = true;
            //     decision.reason = ArmDisarmDecision::OK;
            //     decision.detail = "Already ARMED";

            //     current_arm_decision_ = decision;
            //     arm_decision_publisher_->publish(current_arm_decision_);
            //     return;
            // }

            // sensors check
            auto bat_reason = battery_check();
            if (bat_reason != ArmDisarmDecision::OK){
                decision.reason = bat_reason;
                decision.detail = "battery state is unsafe";
                current_arm_decision_ = decision;

                arm_decision_publisher_->publish(current_arm_decision_);
                return;
            }

            auto range_reason = range_check();
            if (range_reason != ArmDisarmDecision::OK){
                decision.reason = range_reason;
                decision.detail = "unconsistent range data";
                current_arm_decision_ = decision;

                arm_decision_publisher_->publish(current_arm_decision_);
                return;
            }

            // all checks ok
            decision.accepted = true;
            decision.reason = ArmDisarmDecision::OK;
            decision.detail = "ARM accepted";
            current_arm_decision_ = decision;
            
            arm_decision_publisher_->publish(current_arm_decision_);
    };


    BatteryState battery_state_msg_;
    Range range_sensor_msg_;
    // ArmingState current_arm_state_{};
    ArmDisarmDecision current_arm_decision_{};

    bool has_battery_state_{false};
    bool has_range_state_{false};

    rclcpp::Service<AcceptArmDisarm>::SharedPtr service_;
    rclcpp::Subscription<BatteryState>::SharedPtr battery_state_sub_;
    rclcpp::Subscription<Range>::SharedPtr range_sensor_sub_;

    rclcpp::Publisher<ArmDisarmDecision>::SharedPtr arm_decision_publisher_;

};


int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  int result = 0;

  try {
    rclcpp::spin(std::make_shared<ArmingManager>());
  } catch (const std::exception & ex) {
    std::cerr << "ArmingManager node error: "
              << ex.what() << '\n';
    result = 1;
  }

  rclcpp::shutdown();
  return result;
}