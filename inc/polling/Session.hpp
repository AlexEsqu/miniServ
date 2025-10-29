#pragma once
#include "FileHandler.hpp"
#include "Router.hpp"
#include <iostream>
#include <ctime>

class Session
{
private:
	size_t _sessionId;
	std::map<std::string,std::string> _cookies;

public:
	Session();
	Session(size_t sessionId);
	~Session();
	Session(const Session &copy);

	std::map<std::string,std::string>& getCookies();

	bool	hasSessionId();
	void	setSessionId(size_t sessionId);
	void 	addCookie(std::string value);

	static size_t	generatePseudoRandomNumber();
};