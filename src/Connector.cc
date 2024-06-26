#include "Connector.h"
#include "Logger.h"
#include "Channel.h"
#include "EventLoop.h"

#include <errno.h>

const int Connector::kMaxRetryDelayMs;

Connector::Connector(EventLoop* loop, const InetAddress& serverAddr)
    : loop_(loop)
    , serverAddr_(serverAddr)
    , connect_(false)
    , state_(kDisconnected)
    , retryDelayMs_(kInitRetryDelayMs)
    {}

Connector::~Connector()
{

}

void Connector::start()
{
    connect_ = true;
    loop_->runInLoop(std::bind(&Connector::startInLoop,this));
}

void Connector::startInLoop()
{
    if(connect_)
    {
        connect();
    }
}

void Connector::stop()
{
    connect_ = false;
    loop_->queueInLoop(std::bind(&Connector::stopInLoop, this));
}

void Connector::stopInLoop()
{
    if(state_ == kConnecting)
    {
        setState(kDisconnected);
        int sockfd = removeAndResetChannel();
        retry(sockfd);
    }
}

 void Connector::connect()
 {
    int sockfd = ::socket(serverAddr_.family(), SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    const sockaddr_in* addr = serverAddr_.getSockAddr();
    int ret = ::connect(sockfd, (sockaddr*)addr,static_cast<socklen_t>(sizeof(sockaddr_in)));
    int savedErrno = (ret == 0) ? 0 : errno;
    LOG_INFO("errno : %d \n", savedErrno);
    switch (savedErrno)
    {
        case 0:
        case EINPROGRESS:
        case EINTR:
        case EISCONN:
            connecting(sockfd);
            break;

        case EAGAIN:
        case EADDRINUSE:
        case EADDRNOTAVAIL:
        case ECONNREFUSED:
        case ENETUNREACH:
            retry(sockfd);
            break;

        case EACCES:
        case EPERM:
        case EAFNOSUPPORT:
        case EALREADY:
        case EBADF:
        case EFAULT:
        case ENOTSOCK:
            LOG_ERROR("connect error in Connector::startInLoop %d" , savedErrno);
            ::close(sockfd);
            break;

        default:
            LOG_ERROR("Unexpected error in Connector::startInLoop %d" ,savedErrno);
            ::close(sockfd);
      // connectErrorCallback_();
            break;
  }
 }


 void Connector::restart()
 {
    setState(kDisconnected);
    retryDelayMs_ = kInitRetryDelayMs;
    connect_ = true;
    startInLoop();
 }

 void Connector::connecting(int sockfd)
 {
    setState(kConnecting);
    channel_.reset(new Channel(loop_, sockfd));
    channel_->setWriteCallback(std::bind(&Connector::handleWrite, this)); 
    channel_->setErrorCallback(std::bind(&Connector::handleError, this)); 
    channel_->enableWriting();
    channel_->enableWriting();

 }

 int Connector::removeAndResetChannel()
 {
    channel_->disableAll();
    channel_->remove();
    int sockfd = channel_->fd();
    loop_->queueInLoop(std::bind(&Connector::resetChannel, this));
    return sockfd;
 }

 void Connector::resetChannel()
 {
    channel_.reset();
 }

 void Connector::handleWrite()
 {
    if(state_ == kConnecting)
    {
        int sockfd = removeAndResetChannel();
        int optval, err;
        socklen_t optlen = static_cast<socklen_t>(sizeof optval);

        if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
        {
            err = errno;
        }
        else
        {
            err = optval;
        }
        if(err)
        {
            retry(sockfd);
        }
        else
        {
            setState(kConnected);
            if(connect_)
            {
                newConnectionCallback_(sockfd);
            }
            else
            {
                close(sockfd);
            }    
        }
    }
 }

 void Connector::handleError()
 {
    if(state_ == kConnecting)
    {
        int sockfd = removeAndResetChannel();
        retry(sockfd);
    }
 }

 void Connector::retry(int sockfd)
 {
    close(sockfd);
    setState(kDisconnected);
    if(connect_)
    {
        loop_->runAfter(retryDelayMs_/1000.0,
                    std::bind(&Connector::startInLoop, shared_from_this()));
        retryDelayMs_ = std::min(retryDelayMs_ * 2, kMaxRetryDelayMs);
    }
 }