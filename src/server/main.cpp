#include "chatserver.hpp"
#include "chatservice.hpp"
#include <iostream>
#include <signal.h>
void resetHandler(int)
{
	ChatService::instance()->reset();
	std::cout << "ChatService reset!" << std::endl;
	exit(0);
}

int main(int argc, char **argv)
{
	if (argc < 3)
	{
		std::cerr << "command invalid! example: ./ChatServer 127.0.0.1 6000" << std::endl;
		exit(-1);
	}

	char *ip = argv[1];
	uint16_t port = atoi(argv[2]);

	signal(SIGINT, resetHandler);

	EventLoop loop;
	InetAddress addr(ip, port);
	ChatServer server(&loop, addr, "ChatServer");

	server.start();
	loop.loop();

	return 0;
}