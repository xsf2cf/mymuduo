#include "TcpClient.h"
#include "EventLoop.h"

#include <string>
#include <functional>

class EchoClient {
public:
    EchoClient(EventLoop* loop, const InetAddress& serverAddr)
        : client_(loop, serverAddr, "EchoClient") {
        client_.setConnectionCallback(
            std::bind(&EchoClient::onConnection, this, std::placeholders::_1));
        client_.setMessageCallback(
            std::bind(&EchoClient::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    }

    void connect() {
        client_.connect();
    }

private:
    void onConnection(const TcpConnectionPtr& conn) {
        if (conn->connected()) {
            //LOG_INFO << "Connected to " << conn->peerAddress().toIpPort();
            std::string msg = "Hello, Muduo!";
            conn->send(msg);
        } else {
            //LOG_INFO << "Disconnected from " << conn->peerAddress().toIpPort();
        }
    }

    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time) {
        //muduo::string msg(buf->retrieveAllAsString());
        printf("收到服务器的消息: %s \n", buf->retrieveAllAsString().c_str());
        //LOG_INFO << "Received: " << msg;
    }

    TcpClient client_;
};

int main() {
    //LOG_INFO << "Starting EchoClient...";
    EventLoop loop;
    InetAddress serverAddr(8000,"192.168.32.128");  // Connect to server at 127.0.0.1:8080
    EchoClient client(&loop, serverAddr);
    client.connect();
    loop.loop();
    return 0;
}