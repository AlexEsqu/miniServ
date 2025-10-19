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
	bool				isServerSocket(Sockette* socket);

	//----------------- MEMBER FUNCTION ------------------//

	void				addSocket(Sockette& socket);
	void				addServerSocket(ServerSocket& serverSocket);
	void				removeSocket(Sockette* socket);

	void				addPipe(ClientSocket* client, int pipeFd);
	void				removePipe(ClientSocket* client, int pipeFd);

	void				launchEpollListenLoop();
	void				waitForEvents();
	void				processEvents();
	void				updateSocketEvent(Sockette* socket);

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
