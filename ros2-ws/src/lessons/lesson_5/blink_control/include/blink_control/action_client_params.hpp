#ifndef PARAMS_HPP
#define PARAMS_HPP

#include <string>


namespace defaults {
    inline constexpr int HALF_PERIOD_MS = 250;
    inline constexpr int MIN_HALF_PERIOD_MS = 25;
    inline constexpr int CYCLES = 20;
}

namespace keys{
    inline const std::string HALF_PERIOD_MS = "hal_period_ms";
    inline const std::string CYCLES = "cycles";
}

#endif