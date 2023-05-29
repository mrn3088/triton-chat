#ifndef FRIENDMODEL_H
#define FRIENDMODEL_H

#include "user.hpp"

#include <vector>

// friend information data access object
class FriendModel
{
public:
	// add friend relationship
	void insert(int userid, int friendid);

	// return user's friend list
	std::vector<User> query(int userid);
};
#endif