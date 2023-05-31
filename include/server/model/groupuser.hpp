#ifndef GROUPUSER_H
#define GROUPUSER_H

#include "user.hpp"

class GroupUser : public User
{
public:
	GroupUser &setRole(std::string role)
	{
		this->role = role;
		return *this;
	}

	
	std::string getRole() { return role; }

private:
	std::string role;
};
#endif