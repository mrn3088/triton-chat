#include "chatserver.hpp"
#include "json.hpp"
#include "chatservice.hpp"
#include <functional>
#include <string>
#include <iostream>

using json = nlohmann::json;

ChatServer::ChatServer(EventLoop *loop,
					   const InetAddress &listenAddr,
					   const std::string &nameArg)
	: _server(loop, listenAddr, nameArg),
	  _loop(loop)
{
	_server.setConnectionCallback(std::bind(&ChatServer::onConnection,
											this, std::placeholders::_1));
	_server.setMessageCallback(std::bind(&ChatServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

	_server.setThreadNum(4);
}

void ChatServer::start()
{
	_server.start();
}

void ChatServer::onConnection(const TcpConnectionPtr &conn)
{
	if (!conn->connected())
	{
		ChatService::instance()->clientCloseException(conn);
		conn->shutdown();
	}
}

void ChatServer::onMessage(const TcpConnectionPtr &conn,
						   Buffer *buffer,
						   Timestamp time)
{
	std::string buf = buffer->retrieveAllAsString();
	// deserialize the json string
	json js = json::parse(buf);
	std::cout << js << std::endl;
	auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());

	// call the callback function
	msgHandler(conn, js, time);
}
