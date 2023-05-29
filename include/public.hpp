#ifndef PUBLIC_H
#define PUBLIC_H

/*
public to server and client
*/

enum EnMsgType
{
	LOGIN_MSG = 1,		// login msg
	LOGIN_MSG_ACK,		// login msg ack
	REG_MSG,			// register msg
	REG_MSG_ACK,		// register msg ack
	ONE_CHAT_MSG,		// one chat msg
	ADD_FRIEND_MSG,		// add friend msg
};
#endif