#ifndef PARAMS_HPP
#define PARAMS_HPP

#include <string>
#include <chrono>

namespace defaults {
    inline static constexpr auto REQUEST_TIMEOUT =
    std::chrono::seconds(2);
    inline static constexpr auto REQUEST_TIMER_PERIOD = 
    std::chrono::milliseconds(100);
}

#endif

