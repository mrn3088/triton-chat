#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <string>

using muduo::net::TcpServer;
using muduo::net::EventLoop;
using muduo::net::InetAddress;
using muduo::net::TcpConnectionPtr;
using muduo::net::Buffer;
using muduo::Timestamp;

// class ChatServer
class ChatServer {
public:
	// initialize the server
	ChatServer(EventLoop* loop,
				const InetAddress& listenAddr,
				const std::string& nameArg);
	// start service
	void start();
private:
	// callback to report connection info
	void onConnection(const TcpConnectionPtr&);

	// callback to report message
	void onMessage(const TcpConnectionPtr&,
				Buffer*,
				Timestamp);

	TcpServer _server;
	EventLoop *_loop; 
};

#endif