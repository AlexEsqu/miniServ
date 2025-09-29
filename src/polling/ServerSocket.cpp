# include "ServerSocket.hpp"

//--------------------------- CONSTRUCTORS ----------------------------------//

ServerSocket::ServerSocket(ServerConf conf)
	: _conf(conf)
{
	setPort(conf.getPort());

	_cf = new ContentFetcher();
	_cf->addExecutor(new PHPExecutor());
	_cf->addExecutor(new PythonExecutor());

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

	createEpollInstance();
}

//--------------------------- DESTRUCTORS -----------------------------------//

ServerSocket::~ServerSocket()
{
	for (int i = 0; _eventQueue[i].data.ptr != NULL; ++i) { // horrible loop, to be fixed
		ClientSocket* Connecting = reinterpret_cast<ClientSocket*>(_eventQueue[i].data.ptr);

		// remove from epoll
		epoll_ctl(_epollFd, EPOLL_CTL_DEL, Connecting->getSocketFd(), NULL);
		delete Connecting;
	}

	delete _cf;
}

//---------------------------- OPERATORS ------------------------------------//


//---------------------------- GETTERS --------------------------------------//

const ServerConf&		ServerSocket::getConf() const
{
	return _conf;
}

int						ServerSocket::getEpoll() const
{
	return _epollFd;
}

//------------------------ MEMBER FUNCTIONS ---------------------------------//


void			ServerSocket::createEpollInstance()
{
	_epollFd = epoll_create(1);
	if (_epollFd == -1)
		throw failedEpollCreate();

	_event.events = EPOLLIN | EPOLLOUT | EPOLLET;
	_event.data.fd = getSocketFd();

	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, getSocketFd(), &_event) == -1) {
		throw failedEpollCtl();
	}
}

void			ServerSocket::addSocketToEpoll(ClientSocket& newSocket)
{
	newSocket.setEvent(EPOLLIN | EPOLLOUT | EPOLLET);

	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, newSocket.getSocketFd(), &newSocket.getEvent()) == -1)
	{
		perror("Failed to add server socket to epoll in call to epoll_ctl()");
		throw failedEpollCtl();
	}
}

void			ServerSocket::waitForEvents()
{
	_eventsReadyForProcess = epoll_wait(_epollFd, _eventQueue, MAX_EVENTS, -1);
	if (_eventsReadyForProcess == -1) {
		perror("failed call to epoll_wait():");
		throw failedEpollWait();
	}
}

void			ServerSocket::acceptNewConnection()
{
	// allocating new acccepting socket to be used
	ClientSocket*	Connecting = new ClientSocket(*this);

	Connecting->setSocketNonBlocking();
	Connecting->setEvent(EPOLLIN | EPOLLERR);

	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, Connecting->getSocketFd(), &Connecting->getEvent()) == -1) {
		perror("Failed to accept new connection in call to epoll_ctl()");
		throw failedEpollCtl();
	}

	#ifdef DEBUG
		std::cout << CONNEX_FORMAT("\n++++ Accepted Connection on Socket ";
		std::cout  << Connecting->getSocketFd() << " ++++ \n");
	#endif

}

void			ServerSocket::handleExistingConnection(epoll_event &event)
{
	ClientSocket* Connecting = reinterpret_cast<ClientSocket*>(event.data.ptr);

	// Socket ready to read
	if (event.events & EPOLLIN) {

		try {

			#ifdef DEBUG
				std::cout << "\nClient Socket " << Connecting->getSocketFd() << " ";
			#endif

			// reading the request or request chunk into a Request object
			Connecting->readRequest();

			// if request is complete, fetch content and wrap in HTTP header
			if (Connecting->getRequest()->isComplete())
				_cf->craftSendHTTPResponse(Connecting);

			return;
		}

		catch ( HTTPError &e )
		{
			std::cout << ERROR_FORMAT("\n\n+++++++ HTTP Error Page +++++++ \n\n");
			std::cout << "response is [" << e.getErrorPage() << "\n";
			write(Connecting->getSocketFd(), e.getErrorPage().c_str(), e.getErrorPage().size());
			std::cerr << e.what() << "\n";
		}

		catch ( std::exception &e )
		{
			std::cout << ERROR_FORMAT("\n\n+++++++ Non HTTP Error +++++++ \n\n");
			std::cerr << e.what() << "\n";
			delete Connecting;
			return;
		}

		epoll_ctl(_epollFd, EPOLL_CTL_DEL, Connecting->getSocketFd(), NULL);
		delete Connecting;
	}

	if (event.events & (EPOLLOUT)) {
		std::cout << "Connection requesting write" << std::endl;
	}

	if (event.events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
		std::cout << "Connection closed or error occurred" << std::endl;
		epoll_ctl(_epollFd, EPOLL_CTL_DEL, Connecting->getSocketFd(), NULL);
		delete Connecting;
		return;
	}

}

void			ServerSocket::processEvents()
{
	for (int i = 0; i < _eventsReadyForProcess; ++i) {
		if (_eventQueue[i].data.fd == getSocketFd()) {
			acceptNewConnection();
		} else {
			handleExistingConnection(_eventQueue[i]);
		}
	}
}

void			ServerSocket::launchEpollListenLoop()
{
	waitForEvents();
	processEvents();
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
