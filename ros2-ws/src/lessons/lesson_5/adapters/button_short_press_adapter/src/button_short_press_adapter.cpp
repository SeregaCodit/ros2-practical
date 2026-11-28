#include "button_short_press_adapter/button_short_press_adapter.hpp"
#include <course_utils/module_01/robot_qos.hpp>


ButtonShortPressAdapter::ButtonShortPressAdapter()
: Node("button_short_press_adapter")
{
    param_listener_=
    std::make_shared<short_press_adapter_parameters::ParamListener>(
        get_node_parameters_interface()
    );
    params_ = param_listener_->get_params();

    const auto button_sub_qos = robot_qos::button_pressed_qos();
    rclcpp::SubscriptionOptions sub_options;

    sub_options.event_callbacks.deadline_callback =
        [this](rclcpp::QOSDeadlineRequestedInfo & info){
            on_qos_deadline_callback(info);
        };

    button_sub_ = create_subscription<std_msgs::msg::Bool>(
        "button_state_topic",
        button_sub_qos,
        [this](const std_msgs::msg::Bool::SharedPtr msg){
            on_button_state(msg);
        },
        sub_options
    );

    short_press_publisher_ = create_publisher<std_msgs::msg::Bool>(
        "short_press",
        rclcpp::QoS(10)
    );

    RCLCPP_INFO(
        get_logger(),
        "INITIALIZED"
    );
}

void ButtonShortPressAdapter::on_qos_deadline_callback(
    rclcpp::QOSDeadlineRequestedInfo & info
){
    // just warnin
    RCLCPP_WARN(
        get_logger(),
        "Button state deadline missed. Count: %d",
        info.total_count
    );

}

void ButtonShortPressAdapter::on_button_state(
    const std_msgs::msg::Bool::SharedPtr msg
){

    // front of pushing
    if (
        msg->data
        && !prev_state_
    ){
        button_pressed_at_ = Clock::now();
        prev_state_ = msg->data;
        
        RCLCPP_INFO(
            get_logger(),
            "Button is pressed"
        );
        
        triggered_ = false;
        return;
    }

    // means button is pushed now
    const auto push_preiod_ms = params_.hold_period_ms;

    const auto push_preiod = std::chrono::milliseconds(push_preiod_ms);
    const auto ellapsed = Clock::now() - button_pressed_at_;
    
    std_msgs::msg::Bool pub_msg;

    // button front of realising
    if (
        !msg->data
        && prev_state_
        && !triggered_
    ){
        pub_msg.data = ellapsed <= push_preiod
        ? true
        : false;

        RCLCPP_INFO(
                get_logger(),
                "button is realised. Ellapsed %f seconds",
                std::chrono::duration<double>(ellapsed).count()
        );
        // publish if short press
        if (pub_msg.data){
            short_press_publisher_->publish(pub_msg);
            
            RCLCPP_INFO(
                get_logger(),
               "Publishing short press"
            );
        }

        triggered_ = true;
        prev_state_ = msg->data;
        return;
    }

    prev_state_ = msg->data;
    
}