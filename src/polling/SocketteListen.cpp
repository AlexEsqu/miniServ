# include "SocketteListen.hpp"

//--------------------------- CONSTRUCTORS ----------------------------------//

SocketteListen::SocketteListen(int port)
{
	#ifdef DEBUG
		std::cout << "SocketteListen Constructor called" << std::endl;
	#endif

	setPort(port);

	// Creating socket and file descriptor referring it
	setSocketFd(socket(AF_INET, SOCK_STREAM, 0));
	if (getSocketFd() < 0)
		throw failedSocketCreation();

	// allow socket to be reused and webserv to reload faster with SO_REUSEADDR
	setSocketOption(SO_REUSEADDR);

	// bind serv socket to IP address using the standard IP options and provided port
	bindToIPAddress();

	// set serv sock to listen for incomming connections with max incomming
	setListenMode(10);
}

//--------------------------- DESTRUCTORS -----------------------------------//



//---------------------------- OPERATORS ------------------------------------//



//------------------------ MEMBER FUNCTIONS ---------------------------------//


void			SocketteListen::createEpollInstance()
{
	_epollFd = epoll_create(0);
	if (_epollFd == -1)
		throw failedEpollCreate();
}

void			SocketteListen::attachEpollToSocket()
{
	_event.events = EPOLLIN;
	_event.data.fd = getSocketFd();
	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, getSocketFd(), &_event) == -1) {
		throw failedEpollCtl();
	}
}

void			SocketteListen::waitForEvents()
{
	_eventsReadyForProcess = epoll_wait(_epollFd, _eventQueue, MAX_EVENTS, -1);
	if (_eventsReadyForProcess == -1)
		throw failedEpollWait();
}

void			SocketteListen::acceptNewConnection()
{

}

void			SocketteListen::handleExistingConnection()
{

}


void			SocketteListen::processEvents()
{
	for (int i = 0; i < _eventsReadyForProcess; ++i) {
		if (_eventQueue[i].data.fd == getSocketFd()) {
			acceptNewConnection();
		} else {
			handleExistingConnection();
		}
	}
}

void			SocketteListen::launchEpollListenLoop()
{
	while (1)
	{
		waitForEvents();
		processEvents();
	}
}

//--------------------------- EXCEPTIONS ------------------------------------//

const char*		SocketteListen::failedEpollCreate::what() const throw()
{
	return "ERROR: Failed to create epoll instance in call to epoll_create()";
}

const char*		SocketteListen::failedEpollCtl::what() const throw()
{
	return "ERROR: Failed to modify epoll instance in call to epoll_ctl()";
}

const char*		SocketteListen::failedEpollWait::what() const throw()
{
	return "ERROR: Failed to wait with epoll instance in call to epoll_wait()";
}
