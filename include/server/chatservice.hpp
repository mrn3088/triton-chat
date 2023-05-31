#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <muduo/net/TcpConnection.h>
#include <unordered_map>
#include <functional>
#include <mutex>

#include "usermodel.hpp"
#include "offlinemessagemodel.hpp"
#include "friendmodel.hpp"
#include "groupmodel.hpp"
#include "json.hpp"

using json = nlohmann::json;
using muduo::Timestamp;
using muduo::net::TcpConnectionPtr;

// type of message callback function
using MsgHandler = std::function<void(const TcpConnectionPtr &conn, json &js, Timestamp)>;

class ChatService
{
public:
	// get the singleton instance
	static ChatService *instance();
	// handle login
	void login(const TcpConnectionPtr &conn, json &js, Timestamp time);
	// handle register
	void reg(const TcpConnectionPtr &conn, json &js, Timestamp time);
	// one chat
	void oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time);
	// add friend
	void addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time);
	// create group
	void createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
	// join group
	void addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
	// group chat
	void groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time);
	// obtain msg handler
	MsgHandler getHandler(int msgid);
	// handle client close exception
	void clientCloseException(const TcpConnectionPtr &conn);
	// server error, reset client data
	void reset();

private:
	ChatService();

	// store msg id and corresponding handler
	std::unordered_map<int, MsgHandler> _msgHandlerMap;

	// store online user connection
	std::unordered_map<int, TcpConnectionPtr> _userConnMap;

	// mutex for _userConnMap
	std::mutex _connMutex;

	// offline message model
	OfflineMsgModel _offlineMsgModel;

	// user data access object
	UserModel _userModel;

	// friend data model
	FriendModel _friendModel;

	// group data model
	GroupModel _groupModel;
	
};

#endif