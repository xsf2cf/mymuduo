#include "Timer.h"

std::atomic_int64_t Timer::numCreated_(0);

void Timer::restart(Timestamp now)
{
    if(repeat_)
    {
        expiration_ = addTime(now, interval_);
    }
    else
    {
        expiration_ = Timestamp(0);
    }
}