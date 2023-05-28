#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <muduo/net/TcpConnection.h>
#include <unordered_map>
#include <functional>


#include "usermodel.hpp"
#include "json.hpp"

using json = nlohmann::json;
using muduo::net::TcpConnectionPtr;
using muduo::Timestamp;

// type of message callback function
using MsgHandler = std::function<void(const TcpConnectionPtr &conn, json &js, Timestamp)>;

class ChatService {
public:
	// get the singleton instance
	static ChatService* instance();
	// handle login
	void login(const TcpConnectionPtr &conn, json &js, Timestamp time);
	// handle register
	void reg(const TcpConnectionPtr &conn, json &js, Timestamp time);
	// obtain msg handler
	MsgHandler getHandler(int msgid);
private:
	ChatService();

	// store msg id and corresponding handler
	std::unordered_map<int, MsgHandler> _msgHandlerMap;

	UserModel _userModel;
};

#endif