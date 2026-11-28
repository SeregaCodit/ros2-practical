#pragma once

#include <chrono>
#include <rclcpp/qos.hpp>

namespace robot_qos{

    inline rclcpp::QoS arming_state_qos(){
        return rclcpp::QoS(rclcpp::KeepLast(1))
        .reliable()
        .transient_local()
        .liveliness(
            RMW_QOS_POLICY_LIVELINESS_AUTOMATIC
        )
        .liveliness_lease_duration(
            std::chrono::seconds(2)
        );
    }

} // namespace robot_qos