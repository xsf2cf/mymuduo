#pragma once

#include "noncopyable.h"

class InetAddress;


class Socket : noncopyable
{
public:
    explicit Socket(int sockfd): sockfd_(sockfd) {}
    ~Socket();
    int fd() const { return sockfd_; }
    void bindAddress(const InetAddress& localAddr);
    void listen();
    int accept(InetAddress* peerAddr);
    int connect(int sockfd, const struct sockaddr* addr);
    void shutdownWrite();

    void setTcpNoDelay(bool on);
    void setReuseAddr(bool on);
    void setReusePort(bool on);
    void setKeepAlive(bool on);
private:
    const int sockfd_;
};