#include "TcpServer.h"
//#include "../Logger.h"
//#include "muduo/net/TcpServer.h"
//#include "muduo/net/EventLoop.h"
//#include "muduo/net/InetAddress.h"
//#include "muduo/net/TcpConnection.h"

#include <string>
#include <functional>

//using namespace muduo;    
//using namespace muduo::net;

class EchoServer
{
public:
    EchoServer(EventLoop *loop,
            const InetAddress &addr, 
            const std::string &name)
        : server_(loop, addr, name)
        , loop_(loop)
    {
        // 注册回调函数
        server_.setConnectionCallback(
            std::bind(&EchoServer::onConnection, this, std::placeholders::_1)
        );

        server_.setMessageCallback(
            std::bind(&EchoServer::onMessage, this,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
        );

        // 设置合适的loop线程数量 loopthread
        //server_.setThreadNum(3);
    }
    void start()
    {
        server_.start();
    }

    void setNumThread(int num) { server_.setThreadNum(num);}
private:
    // 连接建立或者断开的回调
    void onConnection(const TcpConnectionPtr &conn)
    {
        printf("收到新消息来自 %s\n", conn->peerAddress().toIpPort().c_str());
        if (conn->connected())
        {
            //LOG_INFO("Connection UP : %s", conn->peerAddress().toIpPort().c_str());
        }
        else
        {
            //LOG_INFO("Connection DOWN : %s", conn->peerAddress().toIpPort().c_str());
        }
    }

    // 可读写事件回调
    void onMessage(const TcpConnectionPtr &conn,
                Buffer *buf,
                Timestamp time)
    {
        //printf("current pid : %d \n" , CurrentThread::tid());
        std::string msg = buf->retrieveAllAsString();
        //printf("msg: %s \n", msg.c_str());
        //conn->send("asdfddsg");
        conn->send(msg+"1");
        //conn->send("01");
        //conn->shutdown(); // 写端   EPOLLHUP =》 closeCallback_
    }

    EventLoop *loop_;
    TcpServer server_;
};

int main()
{
    EventLoop loop;
    InetAddress addr(8000,"192.168.32.128");
    //InetAddress addr("192.168.32.128", 8000);
    EchoServer server(&loop, addr, "EchoServer-01"); // Acceptor non-blocking listenfd  create bind 
    //printf("current pid_t : %d \n" , CurrentThread::tid());
    server.setNumThread(3);
    server.start(); // listen  loopthread  listenfd => acceptChannel => mainLoop =>
    
    loop.loop(); // 启动mainLoop的底层Poller

    return 0;
}