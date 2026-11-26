#include <memory>
#include <functional>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/temperature.hpp"

// Automaticly generated header by building generate_params_library 
#include "cpu_temperature_alarm/cpu_temperature_alarm_params.hpp"

using namespace std::placeholders;


class CpuTemperatureAlarmNotificator : public rclcpp::Node
{
public:
    CpuTemperatureAlarmNotificator()
    : Node("cpu_temperature_alarm_notificator")
    {
        // Declare and validate paremeters ftom yaml scheme
        param_listener_ = 
            std::make_shared<cpu_temperature_alarm_notificator::ParamListener>(
                get_node_parameters_interface()
            );
        
        // snapshot of the validated parameters
        params_ = param_listener_->get_params();

        // Cross-parameter invariant
        if (params_.warning_temperature_c > params_.safe_temperature_c){
            throw std::invalid_argument(
                "warning_temperature_c must be <= safe_temperature_c"
            );
        }
        
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
               
        if ((current_temp >= params_.safe_temperature_c) && in_range_){
            in_range_ = !in_range_;
            RCLCPP_WARN(this->get_logger(), "TEMPERATURE IS TO HIGHT: %.1f", current_temp);
        } else if (current_temp >= params_.warning_temperature_c){
            RCLCPP_WARN_THROTTLE(
                this->get_logger(),  
                *this->get_clock(),
                2000,
                "CPU TEMPERATURE IN DANGER RANGE: %.1f", current_temp
            );
        } else if ((current_temp < params_.safe_temperature_c) && !in_range_){
            in_range_ = !in_range_;
            RCLCPP_INFO(this->get_logger(), "CPU TEMPERATURE IN SAFE RANGE");
        };
    }

    // params
    std::shared_ptr<cpu_temperature_alarm_notificator::ParamListener> param_listener_;
    cpu_temperature_alarm_notificator::Params params_;

    // ros2 interfaces
    rclcpp::Subscription<sensor_msgs::msg::Temperature>::SharedPtr subscription_;

    //internal state
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