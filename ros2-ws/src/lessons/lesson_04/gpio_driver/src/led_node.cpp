#include <gpiod.hpp>
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/bool.hpp>

#include <iostream>
#include <memory>
#include <string>
#include <chrono>

#include "gpio_driver/led_parameters.hpp"

using namespace std::chrono_literals;
using Clock = std::chrono::steady_clock;


class LedNode final : public rclcpp::Node
{
public: 
    LedNode() : Node("led_node"){

        param_listener_ =
            std::make_shared<led_params::ParamListener>(
                get_node_parameters_interface()
            );

        params_ = param_listener_->get_params();
        line_ = params_.line;

        gpiod::chip chip(params_.chip);
        
        request_ = std::make_unique<gpiod::line_request>(
            chip.prepare_request()
                .set_consumer("ros2_led")
                .add_line_settings(
                    line_,
                    gpiod::line_settings()
                        .set_direction(gpiod::line::direction::OUTPUT)  // we will send commans on the line
                        .set_output_value(gpiod::line::value::INACTIVE) // set line value to 0
                
            ).do_request()
        );

        subscription_ = create_subscription<std_msgs::msg::Bool>(
            // best practice - is giving relative names. 
            //It allows to use remapping in launch files and simple reusing node
            "/hardware/led/set", 
            
            rclcpp::QoS(10),
            [this](std_msgs::msg::Bool::SharedPtr msg){
                last_signal_at = Clock::now();

                request_ -> set_value(
                    line_,
                    msg->data
                    ? gpiod::line::value::ACTIVE
                    : gpiod::line::value::INACTIVE
                );

                if (failsafe_triggered_){
                    failsafe_triggered_ = false;
                    RCLCPP_INFO(get_logger(), "signal restored");
                }
                
            }
        );

        signal_failsafe_timeout_ = std::chrono::milliseconds{params_.watchdog_timeout_ms};

        watchdog_signal_ = create_wall_timer(
            100ms,
            [this](){
                if (failsafe_triggered_){
                    return;
                }

                const auto elapsed = Clock::now() - last_signal_at;

                if (elapsed > signal_failsafe_timeout_){
                    led_deactivate();
                    failsafe_triggered_ = true;

                    const int elapsed_s = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() / 1000;

                    RCLCPP_WARN(get_logger(), "FAILSAFE: no commanf for %d seconds\nline GPIO%d switched to INACTIVE", elapsed_s, line_);
                }
            }
        );

        RCLCPP_INFO(get_logger(), "LED GPIO READY at line GPIO%d", line_);

    }
    ~LedNode() override
    {
        if (request_){
            try{
                led_deactivate();
                
                RCLCPP_INFO(get_logger(), "GPIO%d reseted to INACTIVE", line_);

            } catch (const std::exception & ex) { // Destructors must not throw.
                RCLCPP_ERROR(get_logger(), "Safe shutdown failed:\n%s", ex.what());
            }
        }
    }
private:

    void led_deactivate(){
        request_ -> set_value(
            line_,
            gpiod::line::value::INACTIVE
        );
    }

    // parameters
    std::shared_ptr<led_params::ParamListener> param_listener_;
    led_params::Params params_;

    // ros interfaces
    std::unique_ptr<gpiod::line_request> request_;
    rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr subscription_;
    rclcpp::TimerBase::SharedPtr watchdog_signal_;
    
    // internal state
    int line_;
    bool failsafe_triggered_ = false;
    Clock::time_point last_signal_at{Clock::now()};
    std::chrono::milliseconds signal_failsafe_timeout_;
};


int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  int result = 0;

  try {
    rclcpp::spin(std::make_shared<LedNode>());
  } catch (const std::exception & ex) {
    std::cerr << "LED node error: "
              << ex.what() << '\n';
    result = 1;
  }

  rclcpp::shutdown();
  return result;
}