#pragma once

#include "noncopyable.h"
#include "Socket.h"
#include "Channel.h"

#include<functional>

class EventLoop;
class InetAddress;

class Acceptor : noncopyable
{
public:
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddress&)>;
    Acceptor(EventLoop* loop, const InetAddress &lislenAddr, bool reuseport);
    ~Acceptor();

    void setNewConnctionCallback(const NewConnectionCallback& cb) { newConnectionCallback_ = cb; }
    bool listening() const {return listenning_; }
    void listen();
private:
    void handleRead();

    EventLoop* loop_;
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool listenning_;
};