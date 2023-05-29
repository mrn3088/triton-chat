#include "chatservice.hpp"
#include "public.hpp"

#include <muduo/base/Logging.h>
#include <vector>

// method to get the singleton instance
ChatService *ChatService::instance()
{
    static ChatService service; // this is thread-safe
    return &service;
}

// server error, reset client data
void ChatService::reset()
{
    _userModel.resetState();
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

    _msgHandlerMap.insert({ONE_CHAT_MSG,
                           std::bind(&ChatService::oneChat, this, std::placeholders::_1,
                                     std::placeholders::_2, std::placeholders::_3)});
    
    _msgHandlerMap.insert({ADD_FRIEND_MSG,
                           std::bind(&ChatService::addFriend, this, std::placeholders::_1,
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
            // record user connection
            {
                std::lock_guard<std::mutex> lock(_connMutex);
                _userConnMap.insert({id, conn});
            }

            // login success, state offline => online
            user.setState("online");
            _userModel.updateState(user);

            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();

            // query offline message
            std::vector<std::string> vec = _offlineMsgModel.query(id);

            if (!vec.empty())
            {
                response["offlinemsg"] = vec;
                // read offline message, delete offline message
                _offlineMsgModel.remove(id);
            }

            // query friend information
            std::vector<User> userVec = _friendModel.query(id);
            if (!userVec.empty())
            {
                std::vector<std::string> vec2;
                for (User &user : userVec)
                {
                    json js;
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    vec2.push_back(js.dump());
                }
                response["friends"] = vec2;
            }
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

void ChatService::clientCloseException(const TcpConnectionPtr &conn)
{
    User user;
    {
        std::lock_guard<std::mutex> lock(_connMutex);
        for (auto it = _userConnMap.begin(); it != _userConnMap.end(); ++it)
        {
            if (it->second == conn)
            {
                user.setId(it->first);
                _userConnMap.erase(it);
                break;
            }
        }
    }

    if (user.getId() == -1)
    {
        return;
    }

    user.setState("offline");
    _userModel.updateState(user);
}

void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int toid = js["to"].get<int>();
    {
        std::lock_guard<std::mutex> lock(_connMutex);
        auto it = _userConnMap.find(toid);
        if (it != _userConnMap.end())
        {
            // toid online, forward message to toid user
            it->second->send(js.dump());
            return;
        }
    }

    // not online, store offline message
    _offlineMsgModel.insert(toid, js.dump());
}

void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();

    // store friend relationship to database
    _friendModel.insert(userid, friendid);
}