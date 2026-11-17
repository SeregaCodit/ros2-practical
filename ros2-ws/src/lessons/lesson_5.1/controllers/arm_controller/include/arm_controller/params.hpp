#ifndef PARAMS_HPP
#define PARAMS_HPP

#include <string>
#include <chrono>

namespace defaults {
    inline constexpr int STATE_PUB_MS = 100;
    inline static constexpr auto REQUEST_TIMEOUT =
    std::chrono::seconds(2);
    inline static constexpr auto REQUEST_TIMER_PERIOD = 
    std::chrono::milliseconds(100);
}

namespace keys{
    inline const std::string STATE_PUB_MS = "arm_state_pub_ms";
}

#endif

