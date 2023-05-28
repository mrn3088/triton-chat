#ifndef USER_H
#define USER_H

#include <string>

// ORM class for table User
class User
{
public:
	User(int id = -1, std::string name = "", std::string pwd = "", std::string state = "offline")
		: id(id), name(name), password(pwd), state(state) {}
	
	User& setId(const int& id) { this->id = id; return *this; }
	User& setName(const std::string& name) { this->name = name; return *this; }
	User& setPassword(const std::string& pwd) { this->password = pwd; return *this; }
	User& setState(const std::string& state) { this->state = state; return *this; }

	int getId() const { return id; }
	std::string getName() const { return name; }
	std::string getPassword() const { return password; }
	std::string getState() const { return state; }
	

private:
	int id;
	std::string name;
	std::string password;
	std::string state;
};

#endif