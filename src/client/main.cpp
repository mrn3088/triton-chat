#include "json.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <vector>
#include <ctime>
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
void mainMenu();

// send thread
int main(int argc, char **argv)
{
	if (argc < 3)
	{
		std::cerr << "command invalid! example: ./ChatClient 127.0.0.1 6000" << std::endl;
		exit(-1);
	}

	// parse ip and port from command line
	char *ip = argv[1];
	uint16_t port = atoi(argv[2]);

	//

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
								// time + [id] + name + " said: " + msg
								std::cout << js["time"] << " [" << js["id"] << "] "
										  << js["name"] << " said: " << js["msg"] << std::endl;
							}
						}

						// create a thread to receive message from server
						std::thread readTask(readTaskHandler, clientfd);
						readTask.detach();

						// login success, enter main menu
						mainMenu();
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
}

void mainMenu()
{
}