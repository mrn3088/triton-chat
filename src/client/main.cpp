#include "json.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <vector>
#include <ctime>
#include <functional>
#include <unordered_map>
using json = nlohmann::json;

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "group.hpp"
#include "user.hpp"
#include "public.hpp"

// record current user info
User g_currentUser;

// record current user friend list
std::vector<User> g_currentUserFriendList;

// record current user group list
std::vector<Group> g_currentUserGroupList;

// show current user data
void showCurrentUserData();

// receive thread
void readTaskHandler(int clientfd);

// obtain current time
std::string getCurrentTime();

// main menu of the chat client
void mainMenu(int clientfd);

// send thread
int main(int argc, char **argv)
{
	const char *ip;
	uint16_t port;

	// parse ip and port from command line
	if (argc < 3)
	{
		// std::cerr << "command invalid! example: ./ChatClient 127.0.0.1 6000" << std::endl;
		std::cerr << "using default ip and port : 127.0.0.1 6000" << std::endl;
		ip = "127.0.0.1";
		port = 6000;
	}
	else
	{
		ip = argv[1];
		port = atoi(argv[2]);
	}

	// create socket on client side
	int clientfd = socket(AF_INET, SOCK_STREAM, 0);

	if (-1 == clientfd)
	{
		std::cerr << "socket create error!" << std::endl;
		exit(-1);
	}

	// fill in the server info we need to connect to
	sockaddr_in server;
	memset(&server, 0, sizeof(sockaddr_in));

	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = inet_addr(ip);

	if (-1 == connect(clientfd, (sockaddr *)&server, sizeof(sockaddr_in)))
	{
		std::cerr << "connect server error!" << std::endl;
		close(clientfd);
		exit(-1);
	}

	while (true)
	{
		// display main menu, login, register, exit
		std::cout << "=========================" << std::endl;
		std::cout << "1. login" << std::endl;
		std::cout << "2. register" << std::endl;
		std::cout << "3. exit" << std::endl;
		std::cout << "=========================" << std::endl;
		std::cout << "choice: ";
		int choice = 0;
		std::cin >> choice;
		std::cin.get();

		switch (choice)
		{
		case 1: // login
		{
			int id = 0;
			char pwd[50] = {0};
			std::cout << "userid:";
			std::cin >> id;
			std::cin.get();
			std::cout << "password:";
			std::cin.getline(pwd, 50);

			json js;
			js["msgid"] = LOGIN_MSG;
			js["id"] = id;
			js["password"] = pwd;
			std::string request = js.dump();

			int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
			if (len == -1)
			{
				std::cerr << "send login msg error:" << request << std::endl;
			}
			else
			{
				char buffer[1024] = {0};
				len = recv(clientfd, buffer, 1024, 0);
				if (-1 == len)
				{
					std::cerr << "recv login response error" << std::endl;
				}
				else
				{
					json responsejs = json::parse(buffer);
					if (0 != responsejs["errno"].get<int>()) // login failed
					{
						std::cerr << responsejs["errmsg"] << std::endl;
					}
					else
					{
						// login success, record current user info
						g_currentUser.setId(id).setName(responsejs["name"]);
						std::cerr << responsejs << std::endl;
						// record current user friend list
						if (responsejs.contains("friends"))
						{
							std::vector<std::string> vec = responsejs["friends"];
							for (std::string &str : vec)
							{
								json js = json::parse(str);
								User user;
								user.setId(js["id"].get<int>())
									.setName(js["name"])
									.setState(js["state"]);
								g_currentUserFriendList.push_back(user);
							}
						}

						// record current user group list
						if (responsejs.contains("groups"))
						{
							std::vector<std::string> vec1 = responsejs["groups"];
							for (std::string &groupstr : vec1)
							{
								json grpjs = json::parse(groupstr);
								Group group;
								group.setId(grpjs["id"].get<int>())
									.setName(grpjs["groupname"])
									.setDesc(grpjs["groupdesc"]);

								std::vector<std::string> vec2 = grpjs["users"];
								for (std::string &userstr : vec2)
								{
									GroupUser user;
									json js = json::parse(userstr);
									user.setId(js["id"].get<int>())
										.setName(js["name"])
										.setState(js["state"]);
									user.setRole(js["role"]);
									group.getUsers().push_back(user);
								}

								g_currentUserGroupList.push_back(group);
							}
						}

						// show user data
						showCurrentUserData();

						// show current user's offline message, if any
						if (responsejs.contains("offlinemsg"))
						{
							std::vector<std::string> vec = responsejs["offlinemsg"];
							for (std::string &str : vec)
							{
								json js = json::parse(str);
								int msgtype = js["msgid"].get<int>();
								if (ONE_CHAT_MSG == msgtype)
								{
									std::cout << js["time"].get<std::string>() << " [" << js["id"] << "] "
											  << js["name"].get<std::string>() << " said: "
											  << js["msg"].get<std::string>() << std::endl;
									continue;
								}
								else if (GROUP_CHAT_MSG == msgtype)
								{
									std::cout << "group messsage[" << js["groupid"] << "]:"
											  << js["time"].get<std::string>() << " [" << js["id"]
											  << "]" << js["name"].get<std::string>()
											  << " said: " << js["msg"].get<std::string>() << std::endl;
									continue;
								}
							}
						}

						// create a thread to receive message from server
						std::thread readTask(readTaskHandler, clientfd);
						readTask.detach();

						// login success, enter main menu
						mainMenu(clientfd);
					}
				}
			}
		}
		break;
		case 2: // register
		{
			char name[50] = {0};
			char pwd[50] = {0};
			std::cout << "username:";
			std::cin.getline(name, 50);
			std::cout << "password:";
			std::cin.getline(pwd, 50);

			json js;
			js["msgid"] = REG_MSG;
			js["name"] = name;
			js["password"] = pwd;
			std::string request = js.dump();

			int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
			if (len == -1)
			{
				std::cerr << "send reg msg error:" << request << std::endl;
			}
			else
			{
				char buffer[1024] = {0};
				len = recv(clientfd, buffer, 1024, 0);
				if (-1 == len)
				{
					std::cerr << "recv reg response error" << std::endl;
				}
				else
				{
					json responsejs = json::parse(buffer);
					if (0 != responsejs["errno"].get<int>()) // reg failed
					{
						std::cerr << name << " is already exist, register error!" << std::endl;
					}
					else
					{
						std::cout << name << " register success, userid is "
								  << responsejs["id"] << ", do not forget it!" << std::endl;
					}
				}
			}
		}
		break;
		case 3: // quit
			close(clientfd);
			exit(0);
		default:
			std::cerr << "invalid input!" << std::endl;
			break;
		}
	}
}

void showCurrentUserData()
{
	std::cout << "======================login user======================" << std::endl;
	std::cout << "current login user => id:" << g_currentUser.getId() << " name:" << g_currentUser.getName() << std::endl;
	std::cout << "----------------------friend list---------------------" << std::endl;
	if (!g_currentUserFriendList.empty())
	{
		for (User &user : g_currentUserFriendList)
		{
			std::cout << user.getId() << " " << user.getName() << " " << user.getState() << std::endl;
		}
	}
	std::cout << "----------------------group list----------------------" << std::endl;
	if (!g_currentUserGroupList.empty())
	{
		for (Group &group : g_currentUserGroupList)
		{
			std::cout << group.getId() << " " << group.getName() << " " << group.getDesc() << std::endl;
			for (GroupUser &user : group.getUsers())
			{
				std::cout << user.getId() << " " << user.getName() << " " << user.getState()
						  << " " << user.getRole() << std::endl;
			}
		}
	}
	std::cout << "======================================================" << std::endl;
}

void readTaskHandler(int clientfd)
{
	while (true)
	{
		char buffer[1024] = {0};
		int len = recv(clientfd, buffer, 1024, 0);
		if (-1 == len || 0 == len)
		{
			close(clientfd);
			exit(-1);
		}

		// receive message from server, deserialize it
		json js = json::parse(buffer);
		int msgtype = js["msgid"].get<int>();
		if (ONE_CHAT_MSG == msgtype)
		{
			std::cout << js["time"].get<std::string>() << " [" << js["id"] << "] "
					  << js["name"].get<std::string>() << " said: "
					  << js["msg"].get<std::string>() << std::endl;
			continue;
		}
		else if (GROUP_CHAT_MSG == msgtype)
		{
			std::cout << "group messsage[" << js["groupid"] << "]:"
					  << js["time"].get<std::string>() << " [" << js["id"]
					  << "]" << js["name"].get<std::string>()
					  << " said: " << js["msg"].get<std::string>() << std::endl;
			continue;
		}
	}
}

// "help" command handler
void help(int fd = 0, std::string str = "");

// "chat" command handler
void chat(int, std::string);

// "addfriend" command handler
void addfriend(int, std::string);

// "creategroup" command handler
void creategroup(int, std::string);

// "addgroup" command handler
void addgroup(int, std::string);

// "groupchat" command handler
void groupchat(int, std::string);

// "quit" command handler
void logout(int, std::string);

// command supported by client
std::unordered_map<std::string, std::string> commandMap{
	{"help", "show all command, format help"},
	{"chat", "one to one chat, format chat:friendid:message"},
	{"addfriend", "add friend, format addfriend:friendid"},
	{"creategroup", "create a group, format creategroup:groupname:groupdesc"},
	{"addgroup", "add a group, format addgroup:groupid"},
	{"groupchat", "chat in a group, format groupchat:groupid:message"},
	{"logout", "logout, format logout"}};

// command handler supported by chat client
std::unordered_map<std::string, std::function<void(int, std::string)>> commandHandlerMap = {
	{"help", help},
	{"chat", chat},
	{"addfriend", addfriend},
	{"creategroup", creategroup},
	{"addgroup", addgroup},
	{"groupchat", groupchat},
	{"logout", logout}};

void mainMenu(int clientfd)
{
	help();

	char buffer[1024] = {0};
	while (true)
	{
		std::cin.getline(buffer, 1024);
		std::string commandbuf(buffer);
		std::string command;
		int idx = commandbuf.find(":");
		if (-1 == idx)
		{
			command = commandbuf;
		}
		else
		{
			command = commandbuf.substr(0, idx);
		}
		auto it = commandHandlerMap.find(command);
		if (it == commandHandlerMap.end())
		{
			std::cerr << "invalid input command!" << std::endl;
			continue;
		}

		it->second(clientfd, commandbuf.substr(idx + 1, commandbuf.size() - idx));
	}
}

void help(int, std::string)
{
	std::cout << "command list:" << std::endl;
	for (const auto &p : commandMap)
	{
		std::cout << p.first << " : " << p.second << std::endl;
	}
}

void addfriend(int clientfd, std::string str)
{
	int friendid = atoi(str.c_str());
	json js;
	js["msgid"] = ADD_FRIEND_MSG;
	js["id"] = g_currentUser.getId();
	js["friendid"] = friendid;
	std::string buffer = js.dump();

	int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
	if (-1 == len)
	{
		std::cerr << "send addfriend msg error: " << buffer << std::endl;
	}
}

void chat(int clientfd, std::string str)
{
	int idx = str.find(":");
	if (-1 == idx)
	{
		std::cerr << "chat command invalid!" << std::endl;
		return;
	}

	int friendid = atoi(str.substr(0, idx).c_str());
	std::string msg = str.substr(idx + 1, str.size() - idx);

	json js;
	js["msgid"] = ONE_CHAT_MSG;
	js["id"] = g_currentUser.getId();
	js["name"] = g_currentUser.getName();
	js["toid"] = friendid;
	js["msg"] = msg;
	js["time"] = getCurrentTime();
	std::string buffer = js.dump();

	int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
	if (-1 == len)
	{
		std::cerr << "send chat msg error: " << buffer << std::endl;
	}
}

void creategroup(int clientfd, std::string str)
{
	int idx = str.find(":");
	if (-1 == idx)
	{
		std::cerr << "creategroup command invalid!" << std::endl;
		return;
	}
	std::string groupname = str.substr(0, idx);
	std::string groupdesc = str.substr(idx + 1, str.size() - idx);

	json js;
	js["msgid"] = CREATE_GROUP_MSG;
	js["id"] = g_currentUser.getId();
	js["groupname"] = groupname;
	js["groupdesc"] = groupdesc;
	std::string buffer = js.dump();

	int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
	if (-1 == len)
	{
		std::cerr << "send creategroup msg error: " << buffer << std::endl;
	}
}

void addgroup(int clientfd, std::string str)
{
	int groupid = atoi(str.c_str());
	json js;
	js["msgid"] = ADD_GROUP_MSG;
	js["id"] = g_currentUser.getId();
	js["groupid"] = groupid;
	std::string buffer = js.dump();

	int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
	if (-1 == len)
	{
		std::cerr << "send addgroup msg error: " << buffer << std::endl;
	}
}

void groupchat(int clientfd, std::string str)
{
	int idx = str.find(":");
	if (-1 == idx)
	{
		std::cerr << "groupchat command invalid!" << std::endl;
		return;
	}

	int groupid = atoi(str.substr(0, idx).c_str());
	std::string msg = str.substr(idx + 1, str.size() - idx);

	json js;
	js["msgid"] = GROUP_CHAT_MSG;
	js["id"] = g_currentUser.getId();
	js["name"] = g_currentUser.getName();
	js["groupid"] = groupid;
	js["msg"] = msg;
	js["time"] = getCurrentTime();
	std::string buffer = js.dump();

	int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
	if (-1 == len)
	{
		std::cerr << "send groupchat msg error: " << buffer << std::endl;
	}
}

void logout(int, std::string)
{
}

std::string getCurrentTime()
{
	auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	struct tm *ptm = localtime(&tt);
	char date[60] = {0};
	sprintf(date, "%d-%02d-%02d %02d:%02d:%02d",
			(int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday,
			(int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);
	return std::string(date);
}