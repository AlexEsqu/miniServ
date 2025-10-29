#include "Session.hpp"

Session::Session() : _sessionId(0)
{
}
Session::Session(size_t sessionId): _sessionId(sessionId)
{
}

Session::~Session()
{
}

Session::Session(const Session &copy)
{
	this->_sessionId = copy._sessionId;
	this->_cookies = copy._cookies;
}

size_t Session::generatePseudoRandomNumber()
{
	return (std::time(0) >> rand());
}

void Session::setSessionId(size_t sessionId)
{
	this->_sessionId = sessionId;
}

void Session::addCookie(std::string str)
{
	std::string key = str.substr(0, str.find('='));
	std::string value = str.substr(str.find('=') + 1);
	// if (key == "session_id" && _sessionId == 0)
	// 	setSessionId(std::atol(value.c_str()));
	this->_cookies.insert(std::make_pair(key, value));
}

bool Session::hasSessionId()
{
	return (_sessionId == 0);
}

std::map<std::string, std::string> &Session::getCookies()
{
	return this->_cookies;
}
