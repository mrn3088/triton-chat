#include "usermodel.hpp"
#include "db.h"
#include <iostream>

bool UserModel::insert(User &user)
{
	// create sql statement
	char sql[1024] = {0};
	sprintf(sql, "INSERT INTO User(name, password, state) VALUES ('%s', '%s', '%s')",
			user.getName().c_str(), user.getPassword().c_str(), user.getState().c_str());
	
	MySQL mysql;
	if (mysql.connect())
	{
		if (mysql.update(sql)) 
		{
			// get user id
			user.setId(mysql_insert_id(mysql.getConnection()));
			return true;
		}	
	}

	return false;
	
}