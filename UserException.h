#pragma once

class UserException :
	public std::exception
{
public:
	UserException(const std::string m="common exception"):msg(m){};
	~UserException(void){};
	const char *what(){return msg.c_str();};
private:
	std::string msg;
};

