#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/bool.hpp>

#include <chrono>
#include <stdexcept>
#include <iostream>

#include "system_interfaces/msg/arming_state.hpp"

#include "led_arming_indication_controller/led_indication_params.hpp"
#include <course_utils/module_01/robot_qos.hpp>

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
        
        // ARMING STATE RECIVING
        const auto state_qos = robot_qos::arming_state_qos();

        rclcpp::SubscriptionOptions sub_options;

        sub_options.event_callbacks.liveliness_callback =
            [this](rclcpp::QOSLivelinessChangedInfo & info) {
                if (info.alive_count == 0) {
                    state_valid_ = false;
                    publish_gpio_states();

                    RCLCPP_ERROR(
                        get_logger(),
                        "Arm controller is not alive. Failsafe."
                    );
                }
            };

        state_subscription_ = create_subscription<ArmingState>(
            "state_publisher",
            state_qos,
            [this](const ArmingState &msg){
                
                state_valid_ = 
                    msg.state == ArmingState::ARMED ||
                    msg.state == ArmingState::DISARMED;
                
                if  (!state_valid_){
                    RCLCPP_ERROR(
                        get_logger(),
                        "Invalid Arming State Recived"
                    );
                    
                    return;
                }
                
                state_ = msg;

                publish_gpio_states();
            },
            sub_options
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
        Bool green_led_msg;
        Bool red_led_msg;

        const bool armed = 
            state_valid_ &&
            state_.state == ArmingState::ARMED;

        green_led_msg.data = armed;
        red_led_msg.data = !armed;

        green_led_publisher_->publish(green_led_msg);
        red_led_publisher_->publish(red_led_msg);
    }


    std::shared_ptr<led_indication_parameters::ParamListener> param_listener_;

    ArmingState state_;
    bool state_valid_{false};
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
        std::cerr << "led_arming_indication_controller node error:\n"
        << ex.what() << "\n";

        result = 1;
    }

    return result;
}