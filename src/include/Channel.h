#pragma once

#include "noncopyable.h"
#include "Timestamp.h"


#include<functional>
#include<memory>

class EventLoop; //前向声明减少细节暴露

/**
 * Channel 保存了了文件fd和感兴趣的事件
 * 并且可以通过所属的Eventloop向poller注册或者取消监听事件
*/

class Channel : noncopyable
{
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;

    Channel(EventLoop *loop, int fd);
    ~Channel();

    void handleEvent(Timestamp receiveTime);

    void setReadCallback(ReadEventCallback cb) { readCallback_ = cb; }
    void setWriteCallback(EventCallback cb) { writeCallback_ = cb; }
    void setCloseCallback(EventCallback cb) { closeCallback_ = cb; } 
    void setErrorCallback(EventCallback cb) { errorCallback_ = cb; }

    //防止channel被删除后还继续执行回调
    void tie(const std::shared_ptr<void>&);

    int fd() const { return fd_;}
    int events() const { return events_; }
    void set_revents(int revt) { revents_ = revt; }

    //设置事件相关信息
    void enableReading() { events_ |= kReadEvent; update(); }
    void disableReading() { events_ &= ~kReadEvent; update(); }
    void enableWriting() { events_ |= kWriteEvent; update(); }
    void disableWriting() { events_ &= ~kWriteEvent; update(); }
    void disableAll() { events_ = kNoneEvent; update(); }

    bool isNoneEvent() const { return events_ == kNoneEvent; }
    bool isWriting() const { return events_ & kWriteEvent; }
    bool isReading() const { return events_ & kReadEvent; }

    int index() { return index_; }
    void set_index(int idx) { index_ = idx; }

    EventLoop* ownerLoop() { return loop_; }
    void remove();

private:

    void update();
    void handleEventWithGuard(Timestamp receiveTime);

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    const int fd_; //本通道保存的文件描述符
    EventLoop *loop_;//从属的loop
    int events_;//感兴趣的事件
    int revents_;//poller通知发生的事件
    int index_;

    std::weak_ptr<void> tie_;
    bool tied_;

    //根据返回的revents调用对应的回调函数
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
};