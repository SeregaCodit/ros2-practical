#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/bool.hpp>

#include <memory>
#include <string>
#include <iostream>

using namespace std::placeholders;
using namespace std::chrono_literals;


class LedToggle final : public rclcpp::Node
{
public:
    LedToggle()
    : Node("led_toggle_node")
    {
        subscription_ = create_subscription<std_msgs::msg::Bool>(
            "button_state", // remap to target topic
            rclcpp::QoS(10),
            [this](const std_msgs::msg::Bool::SharedPtr msg){
                
                // switch current state on button pressing
                // swich on button realise
                // to swintch on button push invert condition
                // react only in front of signal

                if (!msg->data && prev_state_){
                    current_state_ = !current_state_;

                    RCLCPP_INFO(get_logger(), "switch LED %s -> %s",
                        !current_state_ ? "on" : "off",
                        current_state_ ?  "on" : "off"
                    );
                    
                };
                prev_state_ = msg->data;
            }
        );

        publisher_ = create_publisher<std_msgs::msg::Bool>(
            "set_led", // remap to target topic
            rclcpp::QoS(10) 
        );

        publish_timer_ = create_wall_timer(
            100ms,
            [this](){
                std_msgs::msg::Bool msg;
                msg.data = current_state_;
                publisher_->publish(msg);
            }
        );

        RCLCPP_INFO(get_logger(), "initialized");
    }

private:


    rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr subscription_;
    rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr publisher_;
    
    rclcpp::TimerBase::SharedPtr publish_timer_;

    bool prev_state_{false};
    bool current_state_{false};

};


int main(int argc, char ** argv ){
    int result = 0;
    rclcpp::init(argc, argv);

    try{
        rclcpp::spin(std::make_shared<LedToggle>());
    } catch (const std::exception & ex) {
        std::cerr << "LedToggle Node error:"
                  << ex.what() << "\n";
        
        result = 1;
    };

    rclcpp::shutdown();
    return result;
}