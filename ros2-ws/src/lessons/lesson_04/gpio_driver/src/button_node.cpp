#include <gpiod.hpp>
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/bool.hpp>

#include <course_utils/debouncer.hpp>

#include <iostream>
#include <memory>
#include <string>
#include <chrono>


using namespace std::chrono_literals;

class ButtonNode final : public rclcpp::Node
{
public:
    using Clock = std::chrono::steady_clock;

    ButtonNode() 
    :   Node("button_node"),
        debouncer_{
            Debouncer::Duration{
                declare_parameter<int>("debounce_ms", 30)
            }
        }
    {
        const auto chip_path = declare_parameter<std::string>(
            "chip", "/dev/gpiochip0"
        );
        line_ = declare_parameter<int>("line", 17);

        gpiod::chip chip(chip_path);

        request_ = std::make_unique<gpiod::line_request>(
            chip.prepare_request()
                .set_consumer("ros2_button")
                .add_line_settings(
                    line_,
                    gpiod::line_settings()
                        .set_direction(gpiod::line::direction::INPUT)
                        .set_bias(gpiod::line::bias::PULL_UP)
                ).do_request());

        publisher_ = create_publisher<std_msgs::msg::Bool>(
            "/hardware/button/pressed",
            rclcpp::QoS(10)
        );

        sample_timer_ = create_wall_timer(
            5ms,
            [this]() { sample(); }
        );

        publish_timer_ = create_wall_timer(
            100ms,
            [this](){
                std_msgs::msg::Bool msg;
                msg.data = debouncer_.state();
                publisher_->publish(msg);
            }
        );

        RCLCPP_INFO(get_logger(), "Button GPIO ready at line GPIO%d", line_);
    }   

private:
    void sample(){
        const bool raw_pressed = request_->get_value(line_) == gpiod::line::value::INACTIVE;

        bool changed = debouncer_.update(raw_pressed, Clock::now());

        if (changed){
            RCLCPP_INFO(get_logger(), "button state chenged to %s",
            debouncer_.state()
                ? "pressed"
                : "released"
            );
        }
    }

    Debouncer debouncer_;
    int line_;
    std::unique_ptr<gpiod::line_request> request_;
    rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr sample_timer_;
    rclcpp::TimerBase::SharedPtr publish_timer_;
};


int main(int argc, char ** argv)
{
    int result = 0;
    rclcpp::init(argc, argv);

    try{
        rclcpp::spin(std::make_shared<ButtonNode>());
    } catch (const std::exception & ex) {
        std::cerr << "Button Node error:"
                  << ex.what() << '\n';
        
        result = 1;
    };

    rclcpp::shutdown();
    return result;
}