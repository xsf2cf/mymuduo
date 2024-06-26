#include "TcpClient.h"
#include "Connector.h"
#include "EventLoop.h"

#include<functional>
#include <stdio.h>

void removeTcpConnection(EventLoop* loop, const TcpConnectionPtr& conn)
{
    loop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}

void removeConnector(const ConnectorPtr& connector)
{

}

sockaddr_in getPeerAddress(int sockfd) {
    struct sockaddr_in peeraddr;
    socklen_t addrlen = sizeof(peeraddr);
    if (getpeername(sockfd, (struct sockaddr*)&peeraddr, &addrlen) == -1) {
        perror("getpeername failed");
        //return;
    }
    return peeraddr;
}

sockaddr_in getLocalAddress(int sockfd) {
    struct sockaddr_in localaddr;
    socklen_t addrlen = sizeof(localaddr);
    if (getsockname(sockfd, (struct sockaddr*)&localaddr, &addrlen) == -1) {
        perror("getsockname failed");
        //return;
    }
    return localaddr;
}

TcpClient::TcpClient(EventLoop* loop, const InetAddress& serverAddr, const std::string& nameArg)
    : loop_(loop)
    , connector_(new Connector(loop, serverAddr))
    , name_(nameArg)
    , connectionCallback_()
    , messageCallback_()
    , retry_(false)
    , connect_(true)
    , nextConnId_(1)
{
    connector_->setNewConnectionCallback(std::bind(&TcpClient::newConnection, this, std::placeholders::_1));
}

TcpClient::~TcpClient()
{
    TcpConnectionPtr conn;
    bool unique = false;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        unique = connection_.unique();
        conn = connection_;
    }
    if(conn)
    {
        CloseCallback cb = std::bind(&removeTcpConnection, loop_, std::placeholders::_1);
        loop_->runInLoop(std::bind(&TcpConnection::setCloseCallback, conn, cb));
        if(unique)
        {
            conn->connectDestroyed();
        }

    }
    else
    {
        connector_->stop();
        loop_->runAfter(1, std::bind(&removeConnector, connector_));
    }
    

}

void TcpClient::connect()
{
    connect_ = true;
    connector_->start();
}

void TcpClient::disconnect()
{
    connect_ = false;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if(connection_)
        {
            connection_->shutdown();
        }
    }
}


void TcpClient::stop()
{
    connect_ = false;
    connector_->stop();
}

void TcpClient::newConnection(int sockfd)
{
    InetAddress peerAddr(getPeerAddress(sockfd));
    char buf[32] = {0};
    snprintf(buf, sizeof buf, ":%s#%d", peerAddr.toIpPort().c_str(), nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;

    InetAddress localAddr(getLocalAddress(sockfd));

    TcpConnectionPtr conn(new TcpConnection(loop_, connName, sockfd, localAddr, peerAddr));
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(std::bind(&TcpClient::removeConnection, this, std::placeholders::_1));
    {
        std::lock_guard<std::mutex> lock(mutex_);
        connection_ = conn;
    }
    connection_->connectEstablished();

}


void TcpClient::removeConnection(const TcpConnectionPtr& conn)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        connection_.reset();
    }

    loop_->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    if(retry_ && connect_)
    {
        connector_->restart();
    }
}