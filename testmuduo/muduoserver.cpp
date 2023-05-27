#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <iostream>
#include <functional>
#include <string>

using muduo::net::TcpServer;
using muduo::net::EventLoop;
using muduo::net::InetAddress;
using muduo::net::TcpConnectionPtr;
using muduo::net::ConnectionCallback;
using muduo::net::Buffer;
using muduo::Timestamp;

/**
 * server based on muduo library
 * 1. compose a muduo::net::TcpServer object
 * 2. create a muduo::net::EventLoop pointer
 * 3. register connection callback and message callback
 * 4. set thread number 
 */
class ChatServer {
public:
    ChatServer(EventLoop* loop, // event loop
        const InetAddress& listenAddr, // IP and port
        const std::string& nameArg) // server name
        :_server(loop, listenAddr, nameArg),
        _loop(loop) {
            // register connection callback
            _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this,std::placeholders::_1));

            // register message callback
            _server.setMessageCallback(std::bind(&ChatServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

            // set thread number
            _server.setThreadNum(4); // 1 I/O thread, 3 worker threads
        }

        void start() {
            _server.start();
        }
    
private:

    // handle connection
    void onConnection (const TcpConnectionPtr& conn) {
        if (conn -> connected()) {
            std::cout << conn -> peerAddress().toIpPort() << " -> " << conn -> localAddress().toIpPort() << "state:online" << std::endl;
        } else {
            std::cout << conn -> peerAddress().toIpPort() << " -> " << conn -> localAddress().toIpPort() << "state:offline" << std::endl;
            conn -> shutdown();
        }
    }

    // handle message
    void onMessage(const TcpConnectionPtr& conn, // connection
                    Buffer* buffer, // buffer
                    Timestamp time) { // receive time
        std::string buf = buffer -> retrieveAllAsString();
        std::cout << "recv data: " << buf << " time: " << time.toString() << std::endl;
        conn -> send(buf);
    }
    TcpServer _server;
    EventLoop* _loop; // epoll
};


int main() {
    EventLoop loop; // epoll
    InetAddress addr("127.0.0.1", 6000); // IP and port
    ChatServer server(&loop, addr, "ChatServer"); // server name

    server.start(); // start server
    loop.loop(); // wait for client connection, and handle message 

    return 0;
}