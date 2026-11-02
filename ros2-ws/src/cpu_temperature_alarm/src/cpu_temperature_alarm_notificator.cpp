#include <memory>
#include <functional>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/temperature.hpp"

using namespace std::placeholders;


class CpuTemperatureAlarmNotificator : public rclcpp::Node
{
public:
    CpuTemperatureAlarmNotificator()
    : Node("cpu_temperature_alarm_notificator")
    {
        this->declare_parameter("safe_temperature_c", 80.0);
        this->declare_parameter("warning_temperature_c", 75.0);

        subscription_ = this->create_subscription<sensor_msgs::msg::Temperature>(
            "/robot/diagnostics/cpu_temperature",
            rclcpp::QoS(10),
            std::bind(&CpuTemperatureAlarmNotificator::callback, this, _1)
        );
    }

private:
    void callback(const sensor_msgs::msg::Temperature::SharedPtr msg)
    {
        auto current_temp = msg->temperature;
        const double safe_temperature_c = this->get_parameter("safe_temperature_c").as_double();
        const double warning_temperature_c = this->get_parameter("warning_temperature_c").as_double();

        if (current_temp >= warning_temperature_c){
            RCLCPP_WARN_THROTTLE(
                this->get_logger(),  
                *this->get_clock(),
                2000,
                "CPU TEMPERATURE IN DANGER RANGE"
            );
        } else if ((current_temp >= safe_temperature_c) && in_range_){
            in_range_ = !in_range_;
            RCLCPP_WARN(this->get_logger(), "TEMPERATURE IS TO HIGHT: %.1f", current_temp);
        } else if ((current_temp < safe_temperature_c) && !in_range_){
            in_range_ = !in_range_;
            RCLCPP_INFO(this->get_logger(), "CPU TEMPERATURE IN SAFE RANGE");
        };
    }

    rclcpp::Subscription<sensor_msgs::msg::Temperature>::SharedPtr subscription_;
    bool in_range_ = true;
    bool in_warning_range = false;
};


int main(int argc, char ** argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<CpuTemperatureAlarmNotificator>());
    rclcpp::shutdown();
    return 0;
}