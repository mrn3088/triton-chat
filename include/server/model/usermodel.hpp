#ifndef USERMODEL_H
#define USERMODEL_H

#include "user.hpp"

class UserModel
{
public:
	// insert user to User table
	bool insert(User& user);

	// query user from User table
	User query(int id);

	// update user state
	bool updateState(const User& user);

	// reset user state
	void resetState();
};

#endif