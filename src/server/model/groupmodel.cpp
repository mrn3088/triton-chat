#include "groupmodel.hpp"
#include "db.h"

// create a group
bool GroupModel::createGroup(Group &group)
{
	char sql[1024];
	sprintf(sql, "INSERT INTO AllGroup(groupname, groupdesc) VALUES('%s', '%s')",
			group.getName().c_str(), group.getDesc().c_str());

	MySQL mysql;
	if (mysql.connect())
	{
		if (mysql.update(sql))
		{
			group.setId(mysql_insert_id(mysql.getConnection()));
			return true;
		}
	}

	return false;
}

// join a group
void GroupModel::addGroup(int userid, int groupid, std::string role)
{
	char sql[1024];
	sprintf(sql, "INSERT INTO GroupUser(groupid, userid, grouprole) VALUES(%d, %d, '%s')",
			groupid, userid, role.c_str());

	MySQL mysql;
	if (mysql.connect())
	{
		mysql.update(sql);
	}
}

// query user's group information
std::vector<Group> GroupModel::queryGroups(int userid)
{
	char sql[1024];
	sprintf(sql, "SELECT a.id, a.groupname, a.groupdesc FROM AllGroup a INNER JOIN  \
				GroupUser b on a.id = b.groupid WHERE b.userid = %d",
			userid);

	std::vector<Group> groupVec;

	MySQL mysql;
	if (mysql.connect())
	{
		MYSQL_RES *res = mysql.query(sql);
		if (res != nullptr)
		{
			MYSQL_ROW row;

			while ((row = mysql_fetch_row(res)) != nullptr)
			{
				Group group;
				group.setId(atoi(row[0])).setName(row[1]).setDesc(row[2]);
				groupVec.push_back(group);
			}
			mysql_free_result(res);
		}
	}

	for (Group &group : groupVec)
	{
		sprintf(sql, "SELECT a.id, a.name, a.state, b.grouprole FROM Users a INNER JOIN \
					GroupUser b on b.userid = a.id WHERE b.groupid = %d",
				group.getId());
		
		MYSQL_RES *res = mysql.query(sql);
		if (res != nullptr)
		{
			MYSQL_ROW row;
			while ((row = mysql_fetch_row(res)) != nullptr)
			{
				GroupUser user;
				user.setId(atoi(row[0])).setName(row[1]).setState(row[2]);
				user.setRole(row[3]);
				group.getUsers().push_back(user);
			}
			mysql_free_result(res);
		}
	}
	return groupVec;
}


std::vector<int> GroupModel::queryGroupUsers(int userid, int groupid)
{
	char sql[1024] = {0};
	sprintf(sql, "SELECT userid FROM GroupUser where groupid = %d AND userid <> %d", groupid, userid);

	std::vector<int> idVec;
	MySQL mysql;
	if (mysql.connect())
	{
		MYSQL_RES *res = mysql.query(sql);
		if (res != nullptr)
		{
			MYSQL_ROW row;
			while ((row = mysql_fetch_row(res)) != nullptr)
			{
				idVec.push_back(atoi(row[0]));
			}
			mysql_free_result(res);
		}
	}
	return idVec;
}