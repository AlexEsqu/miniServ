# include "ServerSocket.hpp"

//--------------------------- CONSTRUCTORS ----------------------------------//

ServerSocket::ServerSocket(int port)
{
	#ifdef DEBUG
		std::cout << "ServerSocket Constructor called" << std::endl;
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


void			ServerSocket::createEpollInstance()
{
	_epollFd = epoll_create(1);
	if (_epollFd == -1)
		throw failedEpollCreate();
}

void			ServerSocket::attachEpollToSocket()
{
	_event.events = EPOLLIN;
	_event.data.fd = getSocketFd();
	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, getSocketFd(), &_event) == -1) {
		throw failedEpollCtl();
	}
}

void			ServerSocket::waitForEvents()
{
	_eventsReadyForProcess = epoll_wait(_epollFd, _eventQueue, MAX_EVENTS, -1);
	if (_eventsReadyForProcess == -1)
		throw failedEpollWait();
}

void			ServerSocket::acceptNewConnection(epoll_event &event)
{
	// allocating new acccepting socket to be used
	SocketteAnswer*	Connecting = new SocketteAnswer(*this);

	Connecting->setSocketNonBlocking();

	event.events = EPOLLIN | EPOLLET;
	// event.data.fd = Connecting->getSocketFd();

	// adding new socket pointer as context in the event itself
	event.data.ptr = &Connecting;

	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, Connecting->getSocketFd(), &_event) == -1) {
		throw failedEpollCtl();
	}

	Connecting->readRequest();

	#ifdef DEBUG
		std::cout << "Connection established" << std::endl;
		std::cout << "Fd is " << Connecting->getSocketFd() << std::endl;
		std::cout << Connecting->getRequest() << std::endl;
	#endif

	handleExistingConnection(event);

	// close(Connecting->getSocketFd());
	// delete Connecting;

}

void			ServerSocket::handleExistingConnection(epoll_event &event)
{
	SocketteAnswer* Connecting = reinterpret_cast<SocketteAnswer*>(event.data.ptr);

	std::cout << Connecting->getRequest() << std::endl;

	#ifdef DEBUG
		std::cout << "Connection handled" << std::endl;
	#endif

}


void			ServerSocket::processEvents()
{
	for (int i = 0; i < _eventsReadyForProcess; ++i) {
		if (_eventQueue[i].data.fd == getSocketFd()) {
			acceptNewConnection(_eventQueue[i]);
		} else {
			handleExistingConnection(_eventQueue[i]);
		}
	}
}

void			ServerSocket::launchEpollListenLoop()
{
	createEpollInstance();
	attachEpollToSocket();
	while (1)
	{
		waitForEvents();
		processEvents();
	}
}

//--------------------------- EXCEPTIONS ------------------------------------//

const char*		ServerSocket::failedEpollCreate::what() const throw()
{
	return "ERROR: Failed to create epoll instance in call to epoll_create()";
}

const char*		ServerSocket::failedEpollCtl::what() const throw()
{
	return "ERROR: Failed to modify epoll instance in call to epoll_ctl()";
}

const char*		ServerSocket::failedEpollWait::what() const throw()
{
	return "ERROR: Failed to wait with epoll instance in call to epoll_wait()";
}
