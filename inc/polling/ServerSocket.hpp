#pragma once
#include <iostream>
#include <sys/epoll.h>

#include "ClientSocket.hpp"
#include "ServerConf.hpp"
#include "ContentFetcher.hpp"
#include "HTTPError.hpp"
#include "PHPExecutor.hpp"
#include "PythonExecutor.hpp"

class ServerSocket: public Sockette
{

private:

	int					_epollFd;
	int					_eventsReadyForProcess;
	struct epoll_event	_event;
	struct epoll_event	_eventQueue[MAX_EVENTS];
	const ServerConf	_conf;

public:

	//----------------- CONSTRUCTORS ---------------------//

	ServerSocket(int port);
	ServerSocket(const ServerConf conf);

	//----------------- DESTRUCTOR -----------------------//


	//------------------- OPERATORS ----------------------//


	//--------------------- GETTER -----------------------//

	const ServerConf&	getConf() const;

	//--------------- MEMBER FUNCTIONS -------------------//

	void				createEpollInstance();
	void				attachEpollToSocket();
	void				waitForEvents();
	void				processEvents();
	void				acceptNewConnection(epoll_event &event);
	void				handleExistingConnection(epoll_event &event);
	void				launchEpollListenLoop();
	void				listeningLoop();

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
