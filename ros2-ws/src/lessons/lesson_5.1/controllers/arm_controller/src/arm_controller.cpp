#include <rclcpp/rclcpp.hpp>

#include <std_msgs/msg/bool.hpp>

#include <stdexcept>
#include <chrono>
#include <iostream>

#include "safety_interfaces/msg/arm_disarm_decision.hpp"
#include "safety_interfaces/srv/accept_arm_disarm.hpp"
#include "system_interfaces/msg/arming_state.hpp"

#include "arm_controller/params.hpp"


using ArmDisarmDecision = safety_interfaces::msg::ArmDisarmDecision;
using ArmingState = system_interfaces::msg::ArmingState;
using AcceptArmDisarm = safety_interfaces::srv::AcceptArmDisarm;

using namespace std::chrono;

class ArmController final : public rclcpp::Node
{
public:
    ArmController()
    : Node("arm_controller")
    {   
        
        declare_parameter<int64_t>(
            keys::STATE_PUB_MS,
            defaults::STATE_PUB_MS
        );
        const auto state_pub_ms = get_parameter(
            keys::STATE_PUB_MS
        ).as_int();        

        state_.stamp = now();
        state_.revision = 0;
        state_.state = ArmingState::DISARMED;

        arm_disarm_client_ = create_client<AcceptArmDisarm>(
            "arm_disarm_manager"
        );

        toggle_state_sub_ = create_subscription<std_msgs::msg::Bool>(
            "toggle_state",
            rclcpp::QoS(10),
            [this](const std_msgs::msg::Bool &msg){
                // toggle state request 
                if (msg.data){
                    send_arm_disarm_request();
                }
                
            }
        );

        // ARMING STATE
        arming_state_pub_ = create_publisher<system_interfaces::msg::ArmingState>(
            "arming_state",
            rclcpp::QoS(10)
        );

        arming_state_pub_timer_ = create_wall_timer(
            std::chrono::milliseconds(state_pub_ms),
            [this](){
                arming_state_pub_->publish(state_);
            }
        );
        RCLCPP_INFO(get_logger(), "INITIALIZED");
    }

private:
    void send_arm_disarm_request(){
        auto request = std::make_shared<AcceptArmDisarm::Request>();
        
        if (state_.state == ArmingState::ARMED){
            request->command = AcceptArmDisarm::Request::DISARM;
        } else if (state_.state == ArmingState::DISARMED){
            request->command = AcceptArmDisarm::Request::ARM;
        } else {
            RCLCPP_ERROR(get_logger(), "Invalid arming system state");
            return;
        }

        RCLCPP_INFO(get_logger(),
                "sending request to approve %s -> %s",
                state_.state == ArmingState::ARMED ? "ARM" : "DISARM",
                state_.state != ArmingState::ARMED ? "ARM" : "DISARM"
            );

        arm_disarm_client_->async_send_request(
            request,
            [this, request](rclcpp::Client<AcceptArmDisarm>::SharedFuture future){
                try{
                    auto response = future.get();

                    RCLCPP_INFO_STREAM(get_logger(), "ARM/DISARM response:\n"
                        << safety_interfaces::msg::to_yaml(response->decision)
                    );

                    if (response->decision.accepted){
                        state_.state = request->command;
                        state_.stamp = now();
                        state_.revision ++;
                    }

                    RCLCPP_INFO_STREAM(get_logger(), "current state:\n"
                        << system_interfaces::msg::to_yaml(state_)
                    );
                }
                catch (const std::exception & ex){
                    RCLCPP_ERROR(
                        get_logger(),
                        "ARM SEVICE ERROR: %s",
                        ex.what()
                    );
                }
            }
        );
    }


    rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr toggle_state_sub_;

    rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr red_led_pub_;
    rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr green_led_pub_;
    rclcpp::Publisher<system_interfaces::msg::ArmingState>::SharedPtr arming_state_pub_;

    rclcpp::Client<safety_interfaces::srv::AcceptArmDisarm>::SharedPtr arm_disarm_client_;

    rclcpp::TimerBase::SharedPtr arming_state_pub_timer_;

    ArmingState state_{};
};


int main(int argc, char ** argv){
    rclcpp::init(argc, argv);
    int result = 0;

    try{
        rclcpp::spin(std::make_shared<ArmController>());
    } catch (const std::exception & ex) {
        std::cerr << "ArmController node error:\n"
        << ex.what() << "\n";

        result = 1;
    }

    rclcpp::shutdown();
    return result;
}