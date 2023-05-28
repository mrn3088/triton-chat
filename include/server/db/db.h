#ifndef DB_H
#define DB_H

#include <mysql/mysql.h>
#include <string>


// mysql class
class MySQL
{
public:
	MySQL();

	~MySQL();

	// connect to mysql
	bool connect();

	// update method
	bool update(std::string sql);

	// query method, return some result
	MYSQL_RES *query(std::string sql);

private:
	MYSQL *_conn;
};

#endif