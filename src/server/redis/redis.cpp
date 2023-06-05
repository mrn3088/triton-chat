#include "redis.hpp"
#include <iostream>
using namespace std;

Redis::Redis()
	: _publish_context(nullptr), _subscribe_context(nullptr)
{
}

Redis::~Redis()
{
	if (_publish_context != nullptr)
	{
		redisFree(_publish_context);
	}

	if (_subscribe_context != nullptr)
	{
		redisFree(_subscribe_context);
	}
}

bool Redis::connect()
{
	// Context connection responsible for publishing messages
	_publish_context = redisConnect("127.0.0.1", 6379);
	if (nullptr == _publish_context)
	{
		cerr << "connect redis failed!" << endl;
		return false;
	}

	// Context connection responsible for subscribing to messages
	_subscribe_context = redisConnect("127.0.0.1", 6379);
	if (nullptr == _subscribe_context)
	{
		cerr << "connect redis failed!" << endl;
		return false;
	}

	// In a separate thread, listen for events on the channel, and report any messages to the application layer
	thread t([&]()
			 { observer_channel_message(); });
	t.detach();

	cout << "connect redis-server success!" << endl;

	return true;
}

// Publish a message to a specified channel in redis
bool Redis::publish(int channel, string message)
{
	redisReply *reply = (redisReply *)redisCommand(_publish_context, "PUBLISH %d %s", channel, message.c_str());
	if (nullptr == reply)
	{
		cerr << "publish command failed!" << endl;
		return false;
	}
	freeReplyObject(reply);
	return true;
}

// Subscribe to a message on a specified channel in redis
bool Redis::subscribe(int channel)
{
	// The SUBSCRIBE command itself will cause the thread to block waiting for messages in the channel,
	// here it only subscribes to the channel, and does not receive channel messages
	// The reception of channel messages is done in a separate thread in the observer_channel_message function
	// Only responsible for sending commands, not blocking to receive the response message from the redis server,
	// otherwise it would compete with the notifyMsg thread for response resources
	if (REDIS_ERR == redisAppendCommand(this->_subscribe_context, "SUBSCRIBE %d", channel))
	{
		cerr << "subscribe command failed!" << endl;
		return false;
	}
	// redisBufferWrite can cyclically send the buffer until the buffer data is sent (done is set to 1)
	int done = 0;
	while (!done)
	{
		if (REDIS_ERR == redisBufferWrite(this->_subscribe_context, &done))
		{
			cerr << "subscribe command failed!" << endl;
			return false;
		}
	}

	return true;
}

// Unsubscribe to a message on a specified channel in redis
bool Redis::unsubscribe(int channel)
{
	if (REDIS_ERR == redisAppendCommand(this->_subscribe_context, "UNSUBSCRIBE %d", channel))
	{
		cerr << "unsubscribe command failed!" << endl;
		return false;
	}
	// redisBufferWrite can cyclically send the buffer until the buffer data is sent (done is set to 1)
	int done = 0;
	while (!done)
	{
		if (REDIS_ERR == redisBufferWrite(this->_subscribe_context, &done))
		{
			cerr << "unsubscribe command failed!" << endl;
			return false;
		}
	}
	return true;
}

// Receive messages from the subscription channel in a separate thread
void Redis::observer_channel_message()
{
	redisReply *reply = nullptr;
	while (REDIS_OK == redisGetReply(this->_subscribe_context, (void **)&reply))
	{
		// The received subscription message is an array with three elements
		if (reply != nullptr && reply->element[2] != nullptr && reply->element[2]->str != nullptr)
		{
			// Report the messages on the channel to the application layer
			_notify_message_handler(atoi(reply->element[1]->str), reply->element[2]->str);
		}

		freeReplyObject(reply);
	}

	cerr << ">>>>>>>>>>>>> observer_channel_message quit <<<<<<<<<<<<<" << endl;
}

void Redis::init_notify_handler(function<void(int, string)> fn)
{
	this->_notify_message_handler = fn;
}
