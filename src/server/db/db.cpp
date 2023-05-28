#include "db.h"
#include <muduo/base/Logging.h>

// db config
const static std::string server = "127.0.0.1";
const static std::string user = "root";
const static std::string password = "123456";
const static std::string dbname = "chat";

MySQL::MySQL()
	: _conn(mysql_init(nullptr))
{
}

MySQL::~MySQL()
{
	if (_conn != nullptr)
	{
		mysql_close(_conn);
	}
}

// connect to mysql
bool MySQL::connect()
{
	MYSQL *p = mysql_real_connect(_conn, server.c_str(), user.c_str(),
								  password.c_str(), dbname.c_str(), 3306, nullptr, 0);
	if (p != nullptr)
	{
		mysql_query(_conn, "set names gbk");
		LOG_INFO << __FILE__ << ":" << __LINE__ << ":"
				 << "connect mysql success!";
	}
	else
	{
		LOG_INFO << __FILE__ << ":" << __LINE__ << ":"
				 << "connect mysql failed!";
	}
	return p;
}

// update method
bool MySQL::update(std::string sql)
{
	if (mysql_query(_conn, sql.c_str()))
	{
		LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << sql << "update failed!";
		return false;
	}
	return true;
}

// query method, return some result
MYSQL_RES *MySQL::query(std::string sql)
{
	if (mysql_query(_conn, sql.c_str()))
	{
		LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << sql << "query failed!";
		return nullptr;
	}
	return mysql_use_result(_conn);
}

// get connection
MYSQL *MySQL::getConnection()
{
	return _conn;
}