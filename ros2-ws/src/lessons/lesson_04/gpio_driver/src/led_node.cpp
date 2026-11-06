#include <gpiod.hpp>
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/bool.hpp>

#include <iostream>
#include <memory>
#include <string>


class LedNode final : public rclcpp::Node
{
public: 
    LedNode() : Node("led_node"){
        const auto chip_path = declare_parameter<std::string>(
            "chip", "/dev/gpiochip0"
        );
        line_ = declare_parameter<int>("line", 27);

        gpiod::chip chip(chip_path);
        
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
            "/hardware/led/set",
            rclcpp::QoS(10),
            [this](std_msgs::msg::Bool::SharedPtr msg){
                request_ -> set_value(
                    line_,
                    msg->data
                    ? gpiod::line::value::ACTIVE
                    : gpiod::line::value::INACTIVE
                );
            }
        );

        RCLCPP_INFO(get_logger(), "LED GPIO READY at line GPIO%d", line_);

    }
    ~LedNode() override
    {
        if (request_){
            try{
                request_ -> set_value(
                    line_,
                    gpiod::line::value::INACTIVE
                );
                
                RCLCPP_INFO(get_logger(), "GPIO%d reseted to INACTIVE", line_);

            } catch (const std::exception & ex) { // Destructors must not throw.
                RCLCPP_ERROR(get_logger(), "Safe shutdown failed:\n%s", ex.what());
            }
        }
    }
private:
    int line_;
    std::unique_ptr<gpiod::line_request> request_;
    rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr subscription_;
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