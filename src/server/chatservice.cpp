#include "chatservice.hpp"
#include "public.hpp"
#include <muduo/base/Logging.h>

// method to get the singleton instance
ChatService *ChatService::instance()
{
	static ChatService service; // this is thread-safe
	return &service;
}

// register message and corresponding callback handler
ChatService::ChatService()
{
	_msgHandlerMap.insert({LOGIN_MSG,
						   std::bind(&ChatService::login, this, std::placeholders::_1,
									 std::placeholders::_2, std::placeholders::_3)});

	_msgHandlerMap.insert({REG_MSG,
						   std::bind(&ChatService::reg, this, std::placeholders::_1,
									 std::placeholders::_2, std::placeholders::_3)});
}

// obtain message handler according to message id
MsgHandler ChatService::getHandler(int msgid)
{
	// record to error log if msgid does not exist
	auto it = _msgHandlerMap.find(msgid);
	if (it == _msgHandlerMap.end())
	{	
		return [=](const TcpConnectionPtr &conn, json &js, Timestamp time) {
			LOG_ERROR << "msgid: " << msgid << " does not exist!";
		};
	}
	else
	{
		return _msgHandlerMap[msgid];
	}
}

// handle login message 
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
	LOG_INFO << "do login service...";
}

void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
	LOG_INFO << "do reg service...";
}