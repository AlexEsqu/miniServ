#pragma once
#include <iostream>
#include "Sockette.hpp"
#include "sys/epoll.h"

class SocketteListen: public Sockette
{

private:

	int					_epollFd;
	int					_eventsReadyForProcess;
	int					_eventsReady;
	struct epoll_event	_event;
	struct epoll_event	_eventQueue[MAX_EVENTS];

public:

	//----------------- CONSTRUCTORS ---------------------//

	SocketteListen(int port);

	//----------------- DESTRUCTOR -----------------------//


	//------------------- OPERATORS ----------------------//


	//--------------- MEMBER FUNCTIONS -------------------//

	void				createEpollInstance();
	void				attachEpollToSocket();
	void				waitForEvents();
	void				processEvents();
	void				acceptNewConnection();
	void				handleExistingConnection();
	void				launchEpollListenLoop();

	//------------------ EXCEPTIONS ----------------------//

	class failedEpollCreate : public std::exception {
		public :
			const char* what() const throw();
	};

	class failedEpollCtl : public std::exception {
		public :
			const char* what() const throw();
	};

	class failedEpollWait : public std::exception {
		public :
			const char* what() const throw();
	};

};
