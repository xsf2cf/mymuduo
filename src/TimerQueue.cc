
#include "Logger.h"
#include "Timer.h"
#include "EventLoop.h"
#include "TimerId.h"
#include "TimerQueue.h"

#include<strings.h>
#include<sys/timerfd.h>
#include<unistd.h>

int createTimerfd()
{
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);

    if(timerfd < 0)
    {
        LOG_FATAL("Failed int timerfd_create! \n");
    }
    return timerfd;
}

struct timespec howMuchTimeFromNow(Timestamp when)
{
    int64_t microseconds = when.secondsSinceEpoch() - Timestamp::now().secondsSinceEpoch();
    if(microseconds < 100)
    {
        microseconds = 100;
    }
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(microseconds / Timestamp::kMicroSecondsPerSecond);
    ts.tv_nsec = static_cast<long>(1000 * (microseconds % Timestamp::kMicroSecondsPerSecond));
    return ts;
}

void readTimerfd(int timerfd, Timestamp now)
{
    uint64_t howmany;
    ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
    if(n != sizeof howmany)
    {
        LOG_ERROR("TimerQueue::handleRead() reads %d bytes instead of 8", n);
    }
}

void resetTimerfd(int timerfd, Timestamp expiration)
{
    struct itimerspec newValue;
    struct itimerspec oldValue;
    bzero(&newValue, sizeof newValue);
    bzero(&oldValue, sizeof oldValue);
    newValue.it_value = howMuchTimeFromNow(expiration);
    int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
    if(ret)
    {
        LOG_ERROR("timerfd_setti,e()\n");
    }
}

TimerQueue::TimerQueue(EventLoop* loop)
    : loop_(loop)
    , timerfd_(createTimerfd())
    , timerfdChannel_(loop, timerfd_)
    , timers_()
    , callingExpiredTimers_(false)
{
    timerfdChannel_.setReadCallback(std::bind(&TimerQueue::handleRead, this));
    timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue()
{
    timerfdChannel_.disableAll();
    timerfdChannel_.remove();
    ::close(timerfd_);
    for(const Entry& timer : timers_)
    {
        delete timer.second;
    }
}

TimerId TimerQueue::addTimer(TimerCallback cb, Timestamp when, double interval)
{
    Timer* timer = new Timer(std::move(cb), when, interval);
    loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));
    return TimerId(timer, timer->sequence());
}

void TimerQueue::cancel(TimerId timerId)
{
    loop_->runInLoop(std::bind(&TimerQueue::cancelInLoop, this, timerId));
}

void TimerQueue::addTimerInLoop(Timer* timer)
{
    bool earliestChanged = insert(timer);
    if(earliestChanged)
    {
        resetTimerfd(timerfd_, timer->expiration());
    }
}


void TimerQueue::cancelInLoop(TimerId timerId)
{
    ActiveTimer timer(timerId.timer_, timerId.sequence_);
    ActiveTimerSet::iterator it = activeTimers_.find(timer);
    if(it != activeTimers_.end())
    {
        size_t n = timers_.erase(Entry(it->first->expiration(), it->first));
        delete it->first;
        activeTimers_.erase(it);

    }else if(callingExpiredTimers_)
    {
        cancelingTimers_.insert(timer);
    }
}

void TimerQueue::handleRead()
{
    Timestamp now(Timestamp::now());
    readTimerfd(timerfd_, now);
    std::vector<Entry> expired = getExpired(now);

    callingExpiredTimers_ = true;
    cancelingTimers_.clear();
    for(const Entry& it : expired)
    {
        it.second->run();
    }
    callingExpiredTimers_ = false;
    reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
{
    std::vector<Entry> expired;
    Entry sentry(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
    TimerList::iterator end = timers_.lower_bound(sentry);
    std::copy(timers_.begin(), end, back_inserter(expired));
    timers_.erase(timers_.begin(), end);
    for(const Entry& it : expired)
    {
        ActiveTimer timer(it.second, it.second->sequence());
        size_t n = activeTimers_.erase(timer);
    }
    return expired;
}

void TimerQueue::reset(const std::vector<Entry>& expired, Timestamp now)
{
    Timestamp nextExpire;

    for(const Entry& it : expired)
    {
        ActiveTimer timer(it.second, it.second->sequence());
        if(it.second->repeat() && cancelingTimers_.find(timer) == cancelingTimers_.end())
        {
            it.second->restart(now);
            insert(it.second);
        }
        else
        {
            delete it.second;
        }
    }
    if(!timers_.empty())
    {
        nextExpire = timers_.begin()->second->expiration();
    }
    if(nextExpire.valid())
    {
        resetTimerfd(timerfd_, nextExpire);
    }
}

bool TimerQueue::insert(Timer* timer)
{
    bool earliestChanged = false;
    Timestamp when = timer->expiration();
    TimerList::iterator it = timers_.begin();
    if(it == timers_.end() || when < it->first)
    {
        earliestChanged = true;
    }
    {
        std::pair<TimerList::iterator, bool> result = timers_.insert(Entry(when, timer));

    }
    {
        std::pair<ActiveTimerSet::iterator, bool> reult = activeTimers_.insert(ActiveTimer(timer, timer->sequence()));
    }
    return earliestChanged;
}
