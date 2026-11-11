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

using namespace std::chrono_literals;



class ArmingManager final: public rclcpp::Node
{
public:
    ArmingManager()
    : Node ("arm_disarm_manager")
    {   

        // ArmState init
        current_arm_state_.stamp = now();
        current_arm_state_.state = ArmState::STANDBY;
        current_arm_state_.revision = 0;

        // sensors heartbeat
        declare_parameter<int64_t>(keys::HEARTBEAT_TIMEOUT_MS,
             defaults::MIN_HEARTBEAT_TIMEOUT);


        //battery
        declare_parameter<double>(keys::MIN_ARM_VOLTAGE_V,
             defaults::MIN_ARM_VOLTAGE);

        battery_state_sub_ = create_subscription<BatteryState>(
            "battery_state_topic", // define real name with remapping
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
        service_ = create_service<SetArm>(
            "arm_manager",
            [this](const std::shared_ptr<SetArm::Request> request,
                std::shared_ptr<SetArm::Response> response)
                {
                    handle_arm_request(request, response);
                }
        );
        // ArmState publishing
        arm_state_publisher_ = create_publisher<ArmState>(
            "arm_state",
            rclcpp::QoS(10)
        );

        arm_state_pub_timer_ = create_wall_timer(
            100ms,
            [this](){
                arm_state_publisher_->publish(current_arm_state_);
            }
        ); 

        // ArmDecision publishing
        arm_decision_publisher_ = create_publisher<ArmDecision>(
            "arm_decision",
            rclcpp::QoS(10)
        );

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
            return ArmDecision::UPS_UNAVAILABLE;
        }

        if (!check_time_freshness(msg.header)){
            RCLCPP_ERROR(get_logger(),
                "battery state msg is too old"
            );
            return ArmDecision::UPS_UNAVAILABLE;
        }
        
        if (!std::isfinite(msg.voltage) ||
            msg.voltage < 0.0f){
                RCLCPP_ERROR(get_logger(),
                "battery voltage must be > 0.0. Got %f",
                msg.voltage
            );
            return ArmDecision::UPS_INVALID;
        }

        auto min_arm_voltage = get_parameter(keys::MIN_ARM_VOLTAGE_V).as_double();
        if (min_arm_voltage > msg.voltage){
            RCLCPP_ERROR(get_logger(),
                "battery voltage is lower safe thrashold. Got %f, threshold %f",
                msg.voltage, defaults::MIN_ARM_VOLTAGE
            );
            return ArmDecision::LOW_VOLTAGE;
        }

        RCLCPP_INFO(get_logger(), "[OK] - battery check");
        return ArmDecision::OK;
    }


    int range_check(){
        const auto &msg = range_sensor_msg_;
        if (!has_range_state_){
            RCLCPP_ERROR(get_logger(),
                "no range sensor data"
            );
            return ArmDecision::RANGE_SENSOR_UNAVAILABLE;
        }

        if (!check_time_freshness(msg.header)){
            RCLCPP_ERROR(get_logger(),
                "range sensor msg is too old"
            );
            return ArmDecision::RANGE_SENSOR_UNAVAILABLE;
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
            return ArmDecision::RANGE_SENSOR_INVALID;
        }

        auto min_dist = get_parameter(keys::MIN_OBSTACLE_DIST_M).as_double();
        if (min_dist > msg.range){
            RCLCPP_ERROR(get_logger(), 
                "obstacle is too close (%f) m. Min: %f",
                msg.range, min_dist
            );
            return ArmDecision::OBSTACLE_TOO_CLOSE;
        }

        RCLCPP_INFO(get_logger(), "[OK] range check");
        return ArmDecision::OK;
    }


    void update_current_arm_state(const uint8_t target_state){
        
        if (current_arm_state_.state != target_state){
            current_arm_state_.state = target_state;
            current_arm_state_.stamp = now();
            current_arm_state_.revision ++;   
        }

        RCLCPP_INFO_STREAM(get_logger(),
            "arm state updated: \n"
            << safety_interfaces::msg::to_yaml(current_arm_state_)
        );
    }


    void handle_arm_request(const std::shared_ptr<SetArm::Request> request,
        std::shared_ptr<SetArm::Response> response){
            auto &decision = response->decision;
            
            decision.command = request->command;
            decision.accepted = false;

            // command validation
            if (request->command != SetArm::Request::ARM &&
                request->command != SetArm::Request::DISARM){
                    decision.reason = ArmDecision::INVALID_COMMAND;
                    decision.detail = "invalid command " + 
                        std::to_string(request->command);
                    
                    current_arm_decision_ = decision;
                    response->state = current_arm_state_;
                    arm_decision_publisher_->publish(current_arm_decision_);
                    return;
                }

            // DISARM IS UNCONDITIONAL
            if (request->command == SetArm::Request::DISARM){
                decision.accepted = true;
                decision.reason = ArmDecision::OK;
                decision.detail = "DISARM accepted";
                current_arm_decision_ = decision;

                update_current_arm_state(ArmState::STANDBY);

                
                response->state = current_arm_state_;
                arm_state_publisher_->publish(current_arm_state_);
                arm_decision_publisher_->publish(current_arm_decision_);
                return;
            }

            if (current_arm_state_.state == ArmState::ARMED){
                decision.accepted = true;
                decision.reason = ArmDecision::OK;
                decision.detail = "Already ARMED";

                response->state = current_arm_state_;
                arm_state_publisher_->publish(current_arm_state_);

                current_arm_decision_ = decision;
                arm_decision_publisher_->publish(current_arm_decision_);
                return;
            }

            // sensors check
            auto bat_reason = battery_check();
            if (bat_reason != ArmDecision::OK){
                decision.reason = bat_reason;
                decision.detail = "battery state is unsafe";
                current_arm_decision_ = decision;

                response->state = current_arm_state_;
                arm_decision_publisher_->publish(current_arm_decision_);
                return;
            }

            auto range_reason = range_check();
            if (range_reason != ArmDecision::OK){
                decision.reason = range_reason;
                decision.detail = "unconsistent range data";
                current_arm_decision_ = decision;

                response->state = current_arm_state_;
                arm_decision_publisher_->publish(current_arm_decision_);
                return;
            }

            // all checks ok
            decision.accepted = true;
            decision.reason = ArmDecision::OK;
            decision.detail = "ARM accepted";
            current_arm_decision_ = decision;
            
            update_current_arm_state(ArmState::ARMED);
            
            response->state = current_arm_state_;
            arm_state_publisher_->publish(current_arm_state_);
            arm_decision_publisher_->publish(current_arm_decision_);
    };


    BatteryState battery_state_msg_;
    Range range_sensor_msg_;
    ArmState current_arm_state_{};
    ArmDecision current_arm_decision_{};

    bool has_battery_state_{false};
    bool has_range_state_{false};

    rclcpp::Service<SetArm>::SharedPtr service_;
    rclcpp::Subscription<BatteryState>::SharedPtr battery_state_sub_;
    rclcpp::Subscription<Range>::SharedPtr range_sensor_sub_;
    rclcpp::Publisher<ArmState>::SharedPtr arm_state_publisher_;
    rclcpp::Publisher<ArmDecision>::SharedPtr arm_decision_publisher_;

    rclcpp::TimerBase::SharedPtr arm_state_pub_timer_;

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