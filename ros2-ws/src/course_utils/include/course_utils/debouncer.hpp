#pragma once

#include <chrono>


class Debouncer final
{
    
public:
    using Clock = std::chrono::steady_clock;
    using Duration = std::chrono::milliseconds;

    explicit Debouncer(Duration debounce_time)
    : debounce_time_(debounce_time)
    {
        if (debounce_time_ <=  Duration::zero()){
            throw std::invalid_argument("debonce time must be greater than 0");
        };
    }


    bool update(bool raw_state, Clock::time_point now)
    // returns true if stable state changed, otherwise false
    {
        if (raw_state != last_raw_state_){
            last_raw_state_ = raw_state;
            changed_at_ = now;
        };

        if ((raw_state != stable_state_)
            && now - changed_at_ >= debounce_time_){
                stable_state_ = raw_state;
                return true;
        };
        
        return false;
    };

    bool state() const{
        return stable_state_;
    }

private:
    Duration debounce_time_;
    bool last_raw_state_{false};
    bool stable_state_{false};

    Clock::time_point changed_at_{};
};