#include "offlinemessagemodel.hpp"
#include "db.h"

// store offline message
void OfflineMsgModel::insert(int userid, std::string msg)
{
	char sql[1024] = {0};
	sprintf(sql, "INSERT INTO OfflineMessage VALUES(%d, '%s')", userid, msg.c_str());

	MySQL mysql;
	if (mysql.connect())
	{
		mysql.update(sql);
	}
}

void OfflineMsgModel::remove(int userid)
{
	char sql[1024] = {0};
	sprintf(sql, "DELETE FROM OfflineMessage WHERE userid=%d", userid);

	MySQL mysql;
	if (mysql.connect())
	{
		mysql.update(sql);
	}
}

std::vector<std::string> OfflineMsgModel::query(int userid)
{
	char sql[1024] = {0};
	sprintf(sql, "SELECT message FROM OfflineMessage WHERE userid=%d", userid);

	std::vector<std::string> vec;
	MySQL mysql;
	if (mysql.connect())
	{
		MYSQL_RES *res = mysql.query(sql);
		if (res != nullptr)
		{
			MYSQL_ROW row;
			while ((row = mysql_fetch_row(res)) != nullptr)
			{
				vec.push_back(row[0]);
			}

			mysql_free_result(res);
		}
	}
	return vec;
}