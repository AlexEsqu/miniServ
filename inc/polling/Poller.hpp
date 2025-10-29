#pragma once

#include <sys/epoll.h>
#include <string>
#include <algorithm>
#include <set>

#include "Sockette.hpp"
#include "ServerSocket.hpp"


enum e_pollingMode
{
	READING,
	WRITING,
	ONLY_ERROR
};
class Poller
{

private:

	int							_epollFd;
	int							_eventsReadyForProcess;
	struct epoll_event			_eventQueue[MAX_EVENTS];
	std::vector<ServerSocket*>	_serverList;

public:

	//----------------- CONSTRUCTORS ---------------------//

	Poller();

	//----------------- DESTRUCTOR -----------------------//

	~Poller();

	//----------------------- SETTER ---------------------//



	//----------------------- GETTER ---------------------//

	int					getEpoll() const;

	//----------------- MEMBER FUNCTION ------------------//

	void				openServersAndAddToWatchList(std::vector<ServerConf>& serversConfs);

	void				addSocket(Sockette& socket);
	void				addServerSocket(ServerSocket& serverSocket);
	void				removeSocket(Sockette* socket);

	void				addPipe(ClientSocket* client, int pipeFd);
	void				removePipe(ClientSocket* client, int pipeFd);
	void				setPipeToNonBlocking(int pipeFd);

	void				launchEpollListenLoop();
	void				waitForEvents();
	void				processEvents();
	void				updateSocketEvent(Sockette* socket);
	void				setPollingMode(e_pollingMode mode, ClientSocket* socket);

	void				closeServers();

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
