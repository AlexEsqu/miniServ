#include "Session.hpp"

Session::Session() : _sessionId(0)
{
}

Session::~Session()
{
}

Session::Session(const Session &copy)
{
	this->_sessionId = copy._sessionId;
}

size_t Session::generatePseudoRandomNumber()
{ 
	return std::time(0);
}

void Session::setSessionId()
{
	this->_sessionId = generatePseudoRandomNumber();
}

bool Session::hasSessionId()
{
	return (_sessionId == 0);
}
