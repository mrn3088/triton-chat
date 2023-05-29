#include "friendmodel.hpp"
#include "db.h"

// add friend
void FriendModel::insert(int userid, int friendid)
{
	char sql[1024] = {0};
	sprintf(sql, "INSERT INTO Friend VALUES(%d, %d)", userid, friendid);

	MySQL mysql;
	if (mysql.connect())
	{
		mysql.update(sql);
	}
}

std::vector<User> FriendModel::query(int userid)
{
	char sql[1024] = {0};
	sprintf(sql, "SELECT a.id,a.name,a.state \
			FROM Users a INNER JOIN Friend b ON b.friendid = a.id \
			WHERE b.userid = %d",
			userid);
	
	std::vector<User> vec;
	MySQL mysql;
	if (mysql.connect())
	{
		MYSQL_RES *res = mysql.query(sql);
		if (res != nullptr)
		{
			MYSQL_ROW row;
			while ((row = mysql_fetch_row(res)) != nullptr)
			{
				User user;
				user.setId(atoi(row[0]));
				user.setName(row[1]);
				user.setState(row[2]);
				vec.push_back(user);
			}
			mysql_free_result(res);
			
		}
	}
	return vec;
}