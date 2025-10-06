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
	_listeningSockets.insert(socket.getSocketFd());
}

void	Poller::waitForEvents()
{
	_eventsReadyForProcess = epoll_wait(_epollFd, _eventQueue, MAX_EVENTS, -1);
	if (_eventsReadyForProcess == -1) {
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

	if (_listeningSockets.find(socket->getSocketFd()) != _listeningSockets.end())
		_listeningSockets.erase(socket->getSocketFd());
}

void	Poller::updateSocketEvent(Sockette* socket)
{
	if (epoll_ctl(_epollFd, EPOLL_CTL_MOD, socket->getSocketFd(), &socket->getEpollEventsMask()) == -1)
		throw failedEpollCtl();
}

bool 	Poller::isServerSocket(Sockette* socket)
{
	if (_listeningSockets.find(socket->getSocketFd()) != _listeningSockets.end())
		return true;
	else
		return false;
}

void	Poller::processEvents()
{
	for (int i = 0; i < _eventsReadyForProcess; ++i)
	{
		Sockette*	currentSocket = static_cast<Sockette*>(_eventQueue[i].data.ptr);

		if (isServerSocket(currentSocket))
		{
			static_cast<ServerSocket*>(currentSocket)->acceptNewConnection();
		}
		else
		{
			static_cast<ClientSocket*>(currentSocket)->getServer()
				.handleExistingConnection(static_cast<ClientSocket*>(currentSocket), _eventQueue[i]);
		}
	}
}

void	Poller::launchEpollListenLoop()
{
	std::cout << CGI_FORMAT("\n+++++++ Waiting for new request +++++++\n");
	waitForEvents();
	processEvents();
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
