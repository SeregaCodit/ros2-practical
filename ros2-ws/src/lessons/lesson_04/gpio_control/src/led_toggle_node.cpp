#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/bool.hpp>

#include <memory>
#include <string>
#include <iostream>

using namespace std::placeholders;


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

        RCLCPP_INFO(get_logger(), "initialized");
    }

private:


    rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr subscription_;
    rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr publisher_;
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