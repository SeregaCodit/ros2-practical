#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/bool.hpp>

#include <memory>
#include <string>
#include <iostream>

using namespace std::chrono_literals;

using Clock = std::chrono::steady_clock;

class LedToggle final : public rclcpp::Node
{
public:
    LedToggle()
    : Node("led_toggle_node")
    {

        // set up failsafe signal timeout
        const auto timeout_ms = declare_parameter<int64_t>("timeout_ms", 2000);

        if (timeout_ms <= 0){
            throw std::invalid_argument("failsage tiomout must be greater than 0");
        };

        timeout_ = std::chrono::milliseconds{timeout_ms};


        subscription_ = create_subscription<std_msgs::msg::Bool>(
            "button_state", // remap to target topic
            rclcpp::QoS(10),
            [this](const std_msgs::msg::Bool::SharedPtr msg){
                
                // switch current state on button pressing
                // swich on button realise
                // to swintch on button push invert condition
                // react only in front of signal
                updated_at_ = Clock::now();
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
                auto now = Clock::now();
                auto elapsed = now - updated_at_;
                
                std_msgs::msg::Bool msg;

                if (elapsed >= timeout_){
                    // check msg freshness 
                    if (!failsafe_){
                        const int ellapsed = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() / 1000;
                        RCLCPP_WARN(get_logger(), "FAILSAFE: no commanf for %d seconds\nsending safe false commnd", ellapsed);
                        failsafe_ = true;
                        current_state_ = false; 
                        prev_state_ = false;
                    }

                    // msg.data = false;
                } else {
                    // msg.data = current_state_;
                    if (failsafe_){
                        RCLCPP_INFO(get_logger(), "signal restored");
                        failsafe_ = false;
                    };
                }
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
    
    Clock::time_point updated_at_{Clock::now()};
    std::chrono::milliseconds timeout_;
    bool failsafe_ = false;

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