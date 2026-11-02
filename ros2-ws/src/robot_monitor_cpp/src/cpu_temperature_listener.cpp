#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/float32.hpp"


class CpuTemperatureListener : public rclcpp::Node
{
public:
    CpuTemperatureListener()
    : Node("cpu_tremperature_listener")
    {

        RCLCPP_WARN(this->get_logger(), 
            "IF TALKER IS RUNNING BUT YOU CAN'T SEE DATA - CHECK RUNNING TALKER'S VERSION. \
            USE cpu_temperature_listener_v2 with cpu_temperature_publiser_v2" 
        );

        subscription_ = this->create_subscription<std_msgs::msg::Float32>(
            "/robot/diagnostics/cpu_temperature",
            rclcpp::QoS(10),
            [this](std_msgs::msg::Float32::ConstSharedPtr msg)
            {
                RCLCPP_INFO(this->get_logger(), "CPU temperature: %.1f C", msg->data);

            }
        );
    }

private:
    rclcpp::Subscription<std_msgs::msg::Float32>::SharedPtr subscription_;
};


int main(int argc, char ** argv)
{
    rclcpp::init(argc, argv);
    
    auto node = std::make_shared<CpuTemperatureListener>();

    rclcpp::spin(node);

    rclcpp::shutdown();

    return 0;
}