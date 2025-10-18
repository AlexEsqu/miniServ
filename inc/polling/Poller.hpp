#pragma once

#include <sys/epoll.h>
#include <string>
#include <algorithm>
#include <set>

#include "Sockette.hpp"
#include "ServerSocket.hpp"

class Poller
{

private:

	int						_epollFd;
	int						_eventsReadyForProcess;
	struct epoll_event		_eventQueue[MAX_EVENTS];
	std::set<int>			_listeningSockets;

public:

	//----------------- CONSTRUCTORS ---------------------//

	Poller();

	//----------------- DESTRUCTOR -----------------------//

	~Poller();

	//----------------------- SETTER ---------------------//



	//----------------------- GETTER ---------------------//

	int					getEpoll() const;

	//----------------- MEMBER FUNCTION ------------------//

	void				addSocket(Sockette& socket);
	void				addServerSocket(ServerSocket& serverSocket);
	void				addPipe(ClientSocket* client, int pipeFd);
	void				removeSocket(Sockette* socket);
	void				removePipe(int pipeFd);
	void				updateSocketEvent(Sockette* socket);
	void				launchEpollListenLoop();
	void				waitForEvents();
	void				processEvents();
	bool				isServerSocket(Sockette* socket);

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
