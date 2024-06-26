#pragma once


class Timer;
using int64_t = signed long int;

class TimerId 
{
public:
    TimerId() : sequence_(0)
    {}
    TimerId(Timer* timer, int64_t seq) : timer_(timer), sequence_(seq)
    {}

    friend class TimerQueue;
private:
    Timer* timer_;
    int64_t sequence_;
};