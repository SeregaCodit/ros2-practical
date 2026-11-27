#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/bool.hpp>

#include <chrono>
#include <stdexcept>
#include <iostream>

#include "system_interfaces/msg/arming_state.hpp"

#include "led_arming_indication_controller/led_indication_params.hpp"

using ArmingState = system_interfaces::msg::ArmingState;
using Bool = std_msgs::msg::Bool;


class LedArmingIndicationController final : public rclcpp::Node
{
public:
    LedArmingIndicationController()
    : Node("led_arming_indication_controller")
    {   

        param_listener_ =
            std::make_shared<led_indication_parameters::ParamListener>(
                get_node_parameters_interface()
            );
        
        const auto params = param_listener_->get_params();

        state_subscription_ = create_subscription<ArmingState>(
            "state_publisher",
            rclcpp::QoS(10),
            [this](const ArmingState &msg){
                state_ = msg;
                // RCLCPP_INFO_STREAM(
                //     get_logger(),
                //     "got state:\n"
                //     << system_interfaces::msg::to_yaml(msg)
                // );

                publish_gpio_states();
            }
        );

        // PUBLISH TO LED DRIVERS
        green_led_publisher_ = create_publisher<Bool>(
            "set_green_led",
            rclcpp::QoS(10)
        );

        red_led_publisher_ = create_publisher<Bool>(
            "set_red_led",
            rclcpp::QoS(10)
        );

        timer_ = create_wall_timer(
            std::chrono::milliseconds(
                params.state_pub_ms
            ),
            [this](){ publish_gpio_states(); }
        );

        RCLCPP_INFO(
            get_logger(),
            "INITIALIZED"
        );

    }
private:
    void publish_gpio_states(){
        Bool gpio22_msg;
        Bool gpio27_msg;

        if (state_.state == ArmingState::ARMED){
            gpio22_msg.data = true;
            gpio27_msg.data = false;
        } else {
            gpio22_msg.data = false;
            gpio27_msg.data = true;

            if (state_.state != ArmingState::DISARMED){
                RCLCPP_ERROR(
                    get_logger(),
                    "invalid ArmingState"
                );
                
                throw std::invalid_argument("invalid ArmingState");
            }
        } 

        green_led_publisher_->publish(gpio22_msg);
        red_led_publisher_->publish(gpio27_msg);
    }
    std::shared_ptr<led_indication_parameters::ParamListener> param_listener_;

    ArmingState state_;
    rclcpp::Subscription<ArmingState>::SharedPtr state_subscription_;

    rclcpp::Publisher<Bool>::SharedPtr green_led_publisher_;
    rclcpp::Publisher<Bool>::SharedPtr red_led_publisher_;

    rclcpp::TimerBase::SharedPtr timer_;
};


int main(int argc, char ** argv){
    rclcpp::init(argc, argv);
    int result = 0;


    try{
        auto node = std::make_shared<LedArmingIndicationController>();
        rclcpp::spin(node);
    } catch (const std::exception & ex) {
        std::cerr << "ArmController node error:\n"
        << ex.what() << "\n";

        result = 1;
    }

    return result;
}