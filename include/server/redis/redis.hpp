#ifndef REDIS_H
#define REDIS_H

#include <hiredis/hiredis.h>
#include <thread>
#include <functional>

class Redis
{
public:
	Redis();
	~Redis();

	// connect to redis server
	bool connect();

	// publish to a channel
	bool publish(int channel, std::string message);

	// subscribe to a channel
	bool subscribe(int channel);

	// unsubscribe from a channel
	bool unsubscribe(int channel);

	// observer channel message
	void observer_channel_message();

	// set notify message handler
	void init_notify_handler(std::function<void(int, std::string)> fn);

private:

	redisContext *_publish_context;

	redisContext *_subscribe_context;

	std::function<void(int, std::string)> _notify_message_handler;
};
#endif