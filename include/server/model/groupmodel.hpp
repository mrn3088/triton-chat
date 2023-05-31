#ifndef GROUPMODEL_H
#define GROUPMODEL_H

#include "group.hpp"
#include <string>
#include <vector>

class GroupModel
{
public:
	// create group
	bool createGroup(Group &group);

	// join a group
	void addGroup(int userid, int groupid, std::string role);

	// query user's group
	std::vector<Group> queryGroups(int userid);

	// query list of user ids in a group
	std::vector<int> queryGroupUsers(int userid, int groupid);
};

#endif
