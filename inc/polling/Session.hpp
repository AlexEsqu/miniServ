#include "FileHandler.hpp"
#include "Router.hpp"
#include <iostream>
#include <ctime>
class Session
{
private:
	size_t _sessionId;

public:
	Session();
	~Session();
	Session(const Session &copy);

	size_t	assignSessionId();

	size_t	getSessionCookie();
	bool	hasSessionId();
	void	setSessionId();
	static size_t	generatePseudoRandomNumber();
};