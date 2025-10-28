#include "Poller.hpp"

Poller::Poller()
{
	_epollFd = epoll_create(1);
	if (_epollFd == -1)
		throw failedEpollCreate();
}

Poller::~Poller()
{
	close(getEpoll());
}

int		Poller::getEpoll() const
{
	return _epollFd;
}

void	Poller::addSocket(Sockette& socket)
{
	socket.setEpollEventsMask(EPOLLIN | EPOLLERR);

	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, socket.getSocketFd(), &socket.getEpollEventsMask()) == -1)
	{
		perror("Failed to add server socket to epoll in call to epoll_ctl()");
		throw failedEpollCtl();
	}
}

void	Poller::addServerSocket(ServerSocket& socket)
{
	addSocket(socket);
	_serverList.push_back(&socket);
}

// necessary when using epoll to avoid a non filled pipe to make the whole server hang
void	Poller::setPipeToNonBlocking(int pipeFd)
{
	int flags = fcntl(pipeFd, F_GETFL, 0);
	if (flags == -1 || fcntl(pipeFd, F_SETFL, flags | O_NONBLOCK) == -1) {
		perror("Failed to set pipe to non-blocking mode");
		throw std::runtime_error("Failed to set pipe to non-blocking mode");
	}
}

void	Poller::addPipe(ClientSocket* client, int pipeFd)
{
	setPipeToNonBlocking(pipeFd);

	// Add the pipe to the epoll instance
	epoll_event event;
	event.events = EPOLLIN | EPOLLERR;
	event.data.ptr = client;

	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, pipeFd, &event) == -1)
	{
		perror("Failed to add pipe to epoll");
		throw failedEpollCtl();
	}

	// only check the socket fd if an error occurs
	setPollingMode(ONLY_ERROR, client);
}

void	Poller::removePipe(ClientSocket* client, int pipeFd)
{
	// removing pipe fd from the polled fd
	if (epoll_ctl(_epollFd, EPOLL_CTL_DEL, pipeFd, NULL) == -1)
	{
		perror("Failed to remove pipe from epoll");
		throw failedEpollCtl();
	}

	close(pipeFd);

	// setting the socket to writing mode
	setPollingMode(WRITING, client);
}

void Poller::setPollingMode(e_pollingMode mode, ClientSocket* socket)
{
	if (mode == WRITING)
	{
		verboseLog("Setting EPOLLOUT for socket");
		socket->setEpollEventsMask(EPOLLOUT | EPOLLERR);
	}
	else if (mode == READING)
	{
		verboseLog("Setting EPOLLIN for socket");
		socket->setEpollEventsMask(EPOLLIN | EPOLLERR);
	}
	else
	{
		verboseLog("Setting ONLY_ERROR for socket");
		socket->setEpollEventsMask(EPOLLERR);
	}
	updateSocketEvent(socket);
}

void	Poller::waitForEvents()
{
	// waiting on event for 5 seconds (non blocking to check for timeouts)
	_eventsReadyForProcess = epoll_wait(_epollFd, _eventQueue, MAX_EVENTS, 1);
	if (_eventsReadyForProcess == -1)
	{
		if (errno == EINTR)
		{
			std::cout << "epoll_wait interrupted by signal, closing down..." << std::endl;
			_eventsReadyForProcess = 0;
			return;
		}
		else
		{
			perror("failed call to epoll_wait():");
			throw failedEpollWait();
		}
	}
}

void	Poller::removeSocket(Sockette* socket)
{
	#ifdef DEBUG
		std::cout << ERROR_FORMAT("\n++++ Removing Socket ";
		std::cout  << socket->getSocketFd() << " ++++ \n");
	#endif

	if (epoll_ctl(_epollFd, EPOLL_CTL_DEL, socket->getSocketFd(), NULL) == -1)
		throw failedEpollCtl();
}

void	Poller::updateSocketEvent(Sockette* socket)
{
	if (epoll_ctl(_epollFd, EPOLL_CTL_MOD, socket->getSocketFd(), &socket->getEpollEventsMask()) == -1)
		throw failedEpollCtl();
}

void	Poller::processEvents()
{
	for (int i = 0; i < _eventsReadyForProcess; ++i)
	{
		Sockette*	currentSocket = static_cast<Sockette*>(_eventQueue[i].data.ptr);

		currentSocket->handleConnection(_eventQueue[i]);
	}
}

void	Poller::launchEpollListenLoop()
{
	// std::cout << CGI_FORMAT("\n+++++++ Waiting for new request +++++++\n");
	waitForEvents();
	processEvents();

	for (size_t i = 0; i < _serverList.size(); i++)
		_serverList[i]->timeoutIdleClients();
}

//--------------------------- EXCEPTIONS ------------------------------------//

const char*		Poller::failedEpollCreate::what() const throw()
{
	return "ERROR: Failed to create epoll instance in call to epoll_create()";
}

const char*		Poller::failedEpollCtl::what() const throw()
{
	return "ERROR: Failed to modify epoll instance in call to epoll_ctl()";
}

const char*		Poller::failedEpollWait::what() const throw()
{
	return "ERROR: Failed to wait with epoll instance in call to epoll_wait()";
}
