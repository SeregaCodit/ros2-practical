#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/bool.hpp>

#include "button_long_press_adapter/params.hpp"

#include <memory>
#include <chrono>
#include <string>
#include <iostream>

using namespace std::chrono_literals;

using Clock = std::chrono::steady_clock;


class ButtonLongPressAdapter final : public rclcpp::Node{
    // publish True if button pushed over 3 sec
public:
    ButtonLongPressAdapter()
    : Node("button_long_press_adapter")
    {
        declare_parameter(keys::PUSH_PERIOD_MS, defaults::PUSH_PERIOD_MS);

        button_sub_ = create_subscription<std_msgs::msg::Bool>(
            "button_state_topic",
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
                    triggered_ = false;
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
                    RCLCPP_INFO(get_logger(), "Button realised to early: %f s",
                    std::chrono::duration<double>(elapsed).count()
                );
                    return;
                }
                // button holded enougth
                if (msg->data  
                    && prev_state_
                    && !triggered_
                    && elapsed  >= push_period_ms
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
        );

        long_press_publisher_ = create_publisher<std_msgs::msg::Bool>(
            "long_press",
            rclcpp::QoS(10)
        );

        RCLCPP_INFO(get_logger(), "INITIALIZED");
    }

private:
    rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr button_sub_;
    rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr long_press_publisher_;
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