#ifndef PARAMS_HPP
#define PARAMS_HPP

#include <string>


namespace defaults {
    inline constexpr int MIN_HEARTBEAT_TIMEOUT = 100;
    inline constexpr double MIN_ARM_VOLTAGE = 7.2;
    inline constexpr double MIN_OBSTACLE_DIST_M = 0.4;


}

namespace keys{
    inline const std::string HEARTBEAT_TIMEOUT_MS = "heartbeat_timeout_ms";
    inline const std::string MIN_ARM_VOLTAGE_V = "min_arm_voltage_v";
    inline const std::string MIN_OBSTACLE_DIST_M = "min_obstacle_dist_m";

}

#endif

