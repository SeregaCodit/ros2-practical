#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/temperature.hpp"


class CpuTemperatureListenerV2 : public rclcpp::Node
{
public:
    CpuTemperatureListenerV2()
    : Node("cpu_temperature_listener_v2")
    {
        subscription_ = this->create_subscription<sensor_msgs::msg::Temperature>(
            "/robot/diagnostics/cpu_temperature",
            rclcpp::QoS(10),
            [this](sensor_msgs::msg::Temperature::ConstSharedPtr msg)
            {
                RCLCPP_INFO(this->get_logger(), "CPU temperature: %.1f C", msg->temperature);
            }
        );
    }

private:
    rclcpp::Subscription<sensor_msgs::msg::Temperature>::SharedPtr subscription_;
};

int main(int argc, char ** argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<CpuTemperatureListenerV2>());
    rclcpp::shutdown();
    return 0;
}