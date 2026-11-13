#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/bool.hpp>

#include "safety_interfaces/srv/set_arm.hpp"
#include "safety_interfaces/msg/arm_state.hpp"
#include "arm_button_controller/params.hpp"

#include <memory>
#include <chrono>
#include <string>
#include <iostream>

using namespace std::chrono_literals;

using Clock = std::chrono::steady_clock;
using ArmState = safety_interfaces::msg::ArmState;


class ArmButtonController final : public rclcpp::Node{
public:
    ArmButtonController()
    : Node("arm_button_controller")
    {
        declare_parameter(keys::PUSH_PERIOD_MS, defaults::PUSH_PERIOD_MS);


        button_sub_ = create_subscription<std_msgs::msg::Bool>(
            "button_topic",
            rclcpp::QoS(10),
            [this](const std_msgs::msg::Bool::SharedPtr msg){

                const auto elapsed = Clock::now() - button_pressed_at; 
                const auto push_period_ms = std::chrono::milliseconds(
                    get_parameter(keys::PUSH_PERIOD_MS
                ).as_int());
                
                 // front of button pushing
                if (
                    msg->data
                    && !prev_state_
                ){
                    button_pressed_at = Clock::now();
                    prev_state_ = msg->data;
                    switched_ = false;
                    RCLCPP_INFO(get_logger(), "button pushed");
                    return;
                }
                // button realised to early
                if ( 
                    !msg->data
                    && prev_state_
                    && elapsed < push_period_ms
                ){
                    prev_state_ = msg->data;
                    RCLCPP_INFO(get_logger(), "Button realise to early: %f s",
                    std::chrono::duration<double>(elapsed).count()
                );
                    return;
                }
                // button holded enougth
                if (msg->data  
                    && prev_state_
                    && !switched_
                    && elapsed  >= push_period_ms
                ){
                    // send request to server
                    RCLCPP_INFO(get_logger(),
                        "button pressed for %f seconds. Try to change %s -> %s",
                        std::chrono::duration<double>(elapsed).count(),
                        state_ ? "ARMED" : "STBY",
                        state_ ? "STBY" : "ARMED"                       
                    );
                    state_ = !state_;
                    switched_ = true;
                    return;
                } 

                prev_state_ = msg->data;
            }
        );

        RCLCPP_INFO(get_logger(), "INITIALIZED");
    }

private:

    rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr button_sub_;
    Clock::time_point button_pressed_at{};
    bool prev_state_{false};
    bool switched_{false};
    uint8_t state_{ArmState::STANDBY};
};


int main(int argc, char ** argv)
{
    int result = 0;
    rclcpp::init(argc, argv);

    try{
        rclcpp::spin(std::make_shared<ArmButtonController>());
    } catch (const std::exception & ex) {
        std::cerr << "ArmButtonController error:"
                  << ex.what() << '\n';
        
        result = 1;
    };

    rclcpp::shutdown();
    return result;
}