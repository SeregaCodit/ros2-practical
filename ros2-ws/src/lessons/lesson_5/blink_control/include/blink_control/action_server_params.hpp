#ifndef PARAMS_HPP
#define PARAMS_HPP

#include <string>


namespace defaults {
    inline constexpr int GPIO_LINE = 23;
    inline constexpr int MAX_GOAL_CYCLES = 200;
    inline constexpr int MIN_HALF_PERIOD_MS = 25;
    inline constexpr int MAX_HALF_PERIOD_MS = 2000;


}

namespace keys{
    inline const std::string GPIO_LINE = "gpio_line";
    inline const std::string MAX_GOAL_CYCLES = "max_goal_cycles";

}

#endif

