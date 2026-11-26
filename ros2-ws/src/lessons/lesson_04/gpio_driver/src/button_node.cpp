#include <gpiod.hpp>
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/bool.hpp>

#include <course_utils/debouncer.hpp>
#include "gpio_driver/button_parameters.hpp"

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
        param_listener_{
            std::make_shared<button_params::ParamListener>(
                get_node_parameters_interface()
            )
        },
        params_{param_listener_->get_params()},
        debouncer_{
            Debouncer::Duration{
                params_.debounce_ms
            }
        }
    {
        const auto chip_path = params_.chip;
        line_ = params_.line;

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
    
    //parameters
    std::shared_ptr<button_params::ParamListener> param_listener_;
    button_params::Params params_;
    
    // ros interfaces
    std::unique_ptr<gpiod::line_request> request_;
    rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr sample_timer_;
    rclcpp::TimerBase::SharedPtr publish_timer_;

    //internal state
    Debouncer debouncer_;
    int line_;
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