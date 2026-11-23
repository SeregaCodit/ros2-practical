#include "button_short_press_adapter/button_short_press_adapter.hpp"
#include <rclcpp/rclcpp.hpp>

#include <exception>
#include <iostream>
#include <memory>


int main(int argc, char ** argv){
    int result = 0;

    rclcpp::init(argc, argv);

    try{
        auto node = std::make_shared<ButtonShortPressAdapter>();
        rclcpp::spin(node);
    } catch (const std::exception & ex){
        std::cerr
            << "ButtonLongPressAdapter error: "
            << ex.what()
            << '\n';
        
        result = 1;
    }

    rclcpp::shutdown();

    return result;
}