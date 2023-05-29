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


int main() {

	signal(SIGINT, resetHandler);
	EventLoop loop;
	InetAddress addr("127.0.0.1", 6000);
	ChatServer server(&loop, addr, "ChatServer");

	server.start();

	loop.loop();
}