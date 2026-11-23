#include "button_short_press_adapter/button_short_press_adapter.hpp"
#include "button_short_press_adapter/params.hpp"


ButtonShortPressAdapter::ButtonShortPressAdapter()
: Node("button_short_press_adapter")
{
    declare_parameter(
        keys::MAX_PUSH_PERIOD_MS,
        defaults::MAX_PUSH_PERIOD_MS
    );

    button_sub_ = create_subscription<std_msgs::msg::Bool>(
        "button_state_topic",
        rclcpp::QoS(10),
        [this](const std_msgs::msg::Bool::SharedPtr msg){
            on_button_state(msg);
        }
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
    const auto push_preiod_ms = get_parameter(
        keys::MAX_PUSH_PERIOD_MS
    ).as_int();

    if (push_preiod_ms <= 0){
        throw std::invalid_argument("push_preiod_ms must be > 0");
    }

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