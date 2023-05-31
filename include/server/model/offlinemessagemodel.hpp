#ifndef OFFLINEMESSAGEMODEL_H
#define OFFLINEMESSAGEMODEL_H

#include <string>
#include <vector>

class OfflineMsgModel
{
public:
	// store offline message
	void insert(int userid, std::string msg);

	// delete offline message
	void remove(int userid);

	// query offline message
	std::vector<std::string> query(int userid);
};

#endif