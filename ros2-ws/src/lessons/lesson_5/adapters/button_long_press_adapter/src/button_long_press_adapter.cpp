#include <memory>
#include <chrono>
#include <string>
#include <iostream>

#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/bool.hpp>

#include <course_utils/module_01/robot_qos.hpp>

#include "button_long_press_adapter/button_long_press_adapter_params.hpp"



using namespace std::chrono_literals;

using Clock = std::chrono::steady_clock;


class ButtonLongPressAdapter final : public rclcpp::Node{
    // publish True if button pushed over 3 sec
public:
    ButtonLongPressAdapter()
    : Node("button_long_press_adapter")
    {
        params_listener_=
        std::make_shared<long_press_adapter_parameters::ParamListener>(
            get_node_parameters_interface()
        );
        params_ = params_listener_->get_params();
        
        
        const auto button_qos = robot_qos::button_pressed_qos();
        rclcpp::SubscriptionOptions sub_options;

        sub_options.event_callbacks.deadline_callback = 
            [this](rclcpp::QOSDeadlineRequestedInfo & info){
                on_qos_deadline_callback(info);
            };

        button_sub_ = create_subscription<std_msgs::msg::Bool>(
            "button_state_topic",
            button_qos,
            [this](const std_msgs::msg::Bool::SharedPtr msg){
                on_button_state(msg);
            },
            sub_options
        );

        long_press_publisher_ = create_publisher<std_msgs::msg::Bool>(
            "long_press",
            rclcpp::QoS(10)
        );

        RCLCPP_INFO(get_logger(), "INITIALIZED");
    }

private:
    void on_button_state(const std_msgs::msg::Bool::SharedPtr msg){
        const auto elapsed = Clock::now() - button_pressed_at; 
        const auto hold_period_ms = 
            std::chrono::milliseconds(params_.hold_period_ms);
        
            // front of button pushing
        if (
            msg->data
            && !prev_state_
        ){
            button_pressed_at = Clock::now();
            prev_state_ = msg->data;
            triggered_ = false;
            RCLCPP_INFO(get_logger(), "button pushed");
            return;
        }
        // button realised to early
        if ( 
            !msg->data
            && prev_state_
            && elapsed < hold_period_ms
        ){
            prev_state_ = msg->data;
            RCLCPP_INFO(get_logger(), "Button realised to early: %f s",
            std::chrono::duration<double>(elapsed).count()
        );
            return;
        }
        // button holded enougth
        if (msg->data  
            && prev_state_
            && !triggered_
            && elapsed  >= hold_period_ms
        ){
            // publish long_press is true
            RCLCPP_INFO(get_logger(),
                "button pressed for %f seconds. Publishing long_press",
                std::chrono::duration<double>(elapsed).count()                    
            );
            
            std_msgs::msg::Bool is_long_press;
            is_long_press.data = true;
            long_press_publisher_->publish(is_long_press);

            triggered_ = true;
            return;
        } 

        prev_state_ = msg->data;
    }

    void on_qos_deadline_callback(
        rclcpp::QOSDeadlineRequestedInfo & info
    ){
        // just warnin
        RCLCPP_WARN(
            get_logger(),
            "Button state deadline missed. Count: %d",
            info.total_count
        );

    }
    //params
    std::shared_ptr<long_press_adapter_parameters::ParamListener> params_listener_;
    long_press_adapter_parameters::Params params_;

    // ros interfaces
    rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr button_sub_;
    rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr long_press_publisher_;

    // internal state
    Clock::time_point button_pressed_at{};
    bool prev_state_{false};
    bool triggered_{false};
     
};


int main(int argc, char ** argv)
{
    int result = 0;
    rclcpp::init(argc, argv);

    try{
        rclcpp::spin(std::make_shared<ButtonLongPressAdapter>());
    } catch (const std::exception & ex) {
        std::cerr << "ButtonLongPressAdapter error:"
                  << ex.what() << '\n';
        
        result = 1;
    };

    rclcpp::shutdown();
    return result;
}