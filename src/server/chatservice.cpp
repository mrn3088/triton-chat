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
		return [=](const TcpConnectionPtr &conn, json &js, Timestamp time)
		{
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
	int id = js["id"].get<int>();
	std::string pwd = js["password"];

	User user = _userModel.query(id);

	if (user.getId() == id && user.getPassword() == pwd)
	{
		if (user.getState() == "online")
		{
			// user already online, reject login request
			json response;
			response["msgid"] = LOGIN_MSG_ACK;
			response["errno"] = 2;
			response["errmsg"] = "user already online";
			conn->send(response.dump());
		}
		else
		{
			// login success, state offline => online
			user.setState("online");
			_userModel.updateState(user);

			json response;
			response["msgid"] = LOGIN_MSG_ACK;
			response["errno"] = 0;
			response["id"] = user.getId();
			response["name"] = user.getName();
			conn->send(response.dump());
		}
	}
	else
	{
		// login failed
		json response;
		response["msgid"] = LOGIN_MSG_ACK;
		response["errno"] = 1;
		response["errmsg"] = "incorrect user id or password!";
		conn->send(response.dump());
	}
}

void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
	std::string name = js["name"];
	std::string pwd = js["password"];

	User user;
	user.setName(name);
	user.setPassword(pwd);
	bool state = _userModel.insert(user);
	if (state)
	{
		json response;
		response["msgid"] = REG_MSG_ACK;
		response["errno"] = 0;
		response["id"] = user.getId();
		conn->send(response.dump());
	}
	else
	{
		json response;
		response["msgid"] = REG_MSG_ACK;
		response["errno"] = 1;
		conn->send(response.dump());
	}
}