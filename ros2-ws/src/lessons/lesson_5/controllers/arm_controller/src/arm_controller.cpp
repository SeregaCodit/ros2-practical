#include <rclcpp/rclcpp.hpp>

#include <std_msgs/msg/bool.hpp>

#include <stdexcept>
#include <chrono>
#include <iostream>
#include <optional>
#include <cstdint>

#include "safety_interfaces/msg/arm_disarm_decision.hpp"
#include "safety_interfaces/srv/accept_arm_disarm.hpp"
#include "system_interfaces/msg/arming_state.hpp"

#include "arm_controller/params.hpp"
#include "arm_controller/arm_controller_params.hpp"
#include <course_utils/module_01/robot_qos.hpp>


using ArmingState = system_interfaces::msg::ArmingState;
using AcceptArmDisarm = safety_interfaces::srv::AcceptArmDisarm;

using namespace std::chrono;

class ArmController final : public rclcpp::Node
{
public:
    ArmController()
    : Node("arm_controller")
    {   
        param_listener_ = 
            std::make_shared<arm_controller_parameters::ParamListener>(
                get_node_parameters_interface()
            );
        
        const auto params = param_listener_->get_params();

        state_.stamp = now();
        state_.revision = 0;
        state_.state = ArmingState::DISARMED;

        // CLIENT
        arm_disarm_client_ = create_client<AcceptArmDisarm>(
            "arm_disarm_manager"
        );

        request_timeout_timer_ = create_wall_timer(
            defaults::REQUEST_TIMER_PERIOD,
            [this](){ request_timeout_timer_tick(); }
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
        const auto state_qos = robot_qos::arming_state_qos();

        arming_state_pub_ = create_publisher<system_interfaces::msg::ArmingState>(
            "arming_state",
            state_qos
        );

        arming_state_pub_->publish(state_);


        RCLCPP_INFO(get_logger(), "INITIALIZED");
    }

private:
    void request_timeout_timer_tick(){
        // no respone waiting
        if (!request_pending_){
            return;
        }

        // response time is in accepted limits
        if (
            steady_clock::now() - request_started_
            < defaults::REQUEST_TIMEOUT
        ){
            return;
        }

        // response time is to long - remove reques

        if (pending_request_id_) {
            arm_disarm_client_->remove_pending_request(
                *pending_request_id_
            );
        }

        request_pending_ = false;
        pending_request_id_.reset();
        ++request_generation_;

        RCLCPP_ERROR(
            get_logger(),
            "ARM/DISARM accept service request timed out"
        );


    }

    void send_arm_disarm_request(){

        // if server available
        if (!arm_disarm_client_->service_is_ready()){
            RCLCPP_WARN(
                get_logger(),
                "ARM/DISARM accept service is unavailable"
            );
            return;
        }

        // if another request already sended
        if (request_pending_){
            RCLCPP_WARN(
                get_logger(),
                "awaiting for previos request response"
            );
            return;
        }

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
        
        request_pending_ = true;
        request_started_ = steady_clock::now();
        const auto generation = ++request_generation_;
        
        try{
            auto pending = arm_disarm_client_->async_send_request(
                request,
                [this, request, generation](rclcpp::Client<AcceptArmDisarm>::SharedFuture future){
                    
                    if (
                        !request_pending_ ||
                        generation != request_generation_
                    ){
                        return;
                    }

                    request_pending_ = false;
                    pending_request_id_.reset();

                    try{
                        auto response = future.get();

                        RCLCPP_INFO_STREAM(get_logger(), "ARM/DISARM response:\n"
                            << safety_interfaces::msg::to_yaml(response->decision)
                        );

                        if (response->decision.accepted){
                            state_.state =
                                request->command == AcceptArmDisarm::Request::ARM
                                ? ArmingState::ARMED
                                : ArmingState::DISARMED;

                            state_.stamp = now();
                            ++state_.revision;
                            arming_state_pub_->publish(state_);
                        }

                        RCLCPP_INFO_STREAM(get_logger(), "current state:\n"
                            << system_interfaces::msg::to_yaml(state_)
                        );

                        request_pending_ = false;
                    }
                    catch (const std::exception & ex){
                        RCLCPP_ERROR(
                            get_logger(),
                            "ARM SEVICE ERROR: %s",
                            ex.what()
                        );

                        request_pending_ = false;
                    }
                }
            );

            pending_request_id_ = pending.request_id;
        } catch (const std::exception &ex) {
            request_pending_ = false;
            pending_request_id_.reset();
            
            RCLCPP_ERROR(
                get_logger(),
                "Failed to send ARM/DISARM request: %s",
                ex.what()
            );

        }
    }
    std::shared_ptr<arm_controller_parameters::ParamListener> param_listener_;

    rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr toggle_state_sub_;

    rclcpp::Publisher<system_interfaces::msg::ArmingState>::SharedPtr arming_state_pub_;

    rclcpp::Client<safety_interfaces::srv::AcceptArmDisarm>::SharedPtr arm_disarm_client_;

    rclcpp::TimerBase::SharedPtr request_timeout_timer_;

    std::optional<int64_t> pending_request_id_;
    std::chrono::steady_clock::time_point request_started_;

    uint64_t request_generation_{0};

    bool request_pending_{false};

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