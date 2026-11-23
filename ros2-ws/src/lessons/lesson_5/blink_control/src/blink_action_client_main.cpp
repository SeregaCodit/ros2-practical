#include <exception>
#include <memory>
#include <iostream>

#include <rclcpp/rclcpp.hpp>

#include "blink_control/blink_action_client.hpp"


int main(int argc, char ** argv)
{
    int result = 0;
    rclcpp::init(argc, argv);

    try{
        auto node =
            std::make_shared<blink_control::ActionClient>();

        rclcpp::spin(node);
    } catch (const std::exception & ex){
        std::cerr << "Blink ActionClient error:"
            << ex.what() << "\n";

        result = 1;
    }

    rclcpp::shutdown();

    return result;
}
