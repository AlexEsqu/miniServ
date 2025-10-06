# include "ServerSocket.hpp"

//--------------------------- CONSTRUCTORS ----------------------------------//

ServerSocket::ServerSocket(ServerConf conf)
	: _epollFd(-1)
	, _conf(conf)
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

	fcntl(getSocketFd(), F_SETFL, O_NONBLOCK);

	createEpollInstance();
}

//--------------------------- DESTRUCTORS -----------------------------------//

ServerSocket::~ServerSocket()
{
	while (!_clients.empty()) {
		removeConnection(_clients.begin()->second);
	}

	delete _cf;

	close(_epollFd);
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

	_event.events = EPOLLIN | EPOLLERR;
	_event.data.fd = getSocketFd();

	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, getSocketFd(), &_event) == -1) {
		throw failedEpollCtl();
	}
}

void			ServerSocket::addSocketToEpoll(ClientSocket& newSocket)
{
	newSocket.setEvent(EPOLLIN | EPOLLOUT | EPOLLERR);

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

void			ServerSocket::acceptNewConnection()
{
	// allocating new acccepting socket to be used
	ClientSocket*	Connecting = new ClientSocket(*this);

	Connecting->setSocketNonBlocking();
	Connecting->setEvent(EPOLLIN | EPOLLOUT | EPOLLERR);

	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, Connecting->getSocketFd(), &Connecting->getEvent()) == -1) {
		perror("Failed to accept new connection in call to epoll_ctl()");
		delete Connecting;
		throw failedEpollCtl();
	}
	_clients[Connecting->getSocketFd()] = Connecting;

	#ifdef DEBUG
		std::cout << CONNEX_FORMAT("\n++++ Accepted Connection on Socket ";
		std::cout  << Connecting->getSocketFd() << " ++++ \n");
	#endif

}

void			ServerSocket::removeConnection(ClientSocket* clientSocket)
{
	#ifdef DEBUG
		std::cout << ERROR_FORMAT("\n++++ Removing Client Socket ";
		std::cout  << clientSocket->getSocketFd() << " ++++ \n");
	#endif

	if (_clients.find(clientSocket->getSocketFd()) != _clients.end())
		_clients.erase(clientSocket->getSocketFd());
	epoll_ctl(_epollFd, EPOLL_CTL_DEL, clientSocket->getSocketFd(), NULL);
	delete clientSocket;

}

void			ServerSocket::handleExistingConnection(epoll_event &event)
{
	ClientSocket* Connecting = reinterpret_cast<ClientSocket*>(event.data.ptr);

	// Socket ready to read data
	if (event.events & EPOLLIN) {

		// reading the request or request chunk into a Request object
		try
		{
			Connecting->readRequest();

			if (Connecting->getRequest()) {
				std::cout << "Request state: " << Connecting->getRequest()->getParsingState() << std::endl;
				std::cout << "Method: " << Connecting->getRequest()->getMethod() << std::endl;
				std::cout << "URL: " << Connecting->getRequest()->getRequestedURL() << std::endl;
			}


			if (Connecting->getRequest() && Connecting->getRequest()->getParsingState() == PARSING_DONE)
			{
				Response response = _cf->createPage(Connecting->getRequest());
				Connecting->setResponse(response.getHTTPResponse());
			}
		}

		catch ( HTTPError &e )
		{
			std::cout << ERROR_FORMAT("\n\n+++++++ HTTP Error Page +++++++ \n\n");
			std::cerr << e.what() << "\n";
		}

		catch ( std::exception &e )
		{
			std::cout << ERROR_FORMAT("\n\n+++++++ Non HTTP Error +++++++ \n\n");
			std::cerr << e.what() << "\n";
			removeConnection(Connecting);
		}
	}

	// socket ready to receive data
	else if (event.events & EPOLLOUT)
	{
		// if request is complete, fetch content and wrap in HTTP header
		if (Connecting->getRequest() &&
			(Connecting->getRequest()->getParsingState() == PARSING_DONE
				|| Connecting->getRequest()->getStatus().getStatusCode() >= 400))
		{
			Connecting->sendResponse();

			if (Connecting->getRequest()->isKeepAlive())
				Connecting->resetRequest();
			else
				removeConnection(Connecting);
		}
	}

	// Socket not doing so well
	else if (event.events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
		std::cout << "Connection closed or error occurred" << std::endl;
		removeConnection(Connecting);
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
	// std::cout << CGI_FORMAT("\n+++++++ Waiting for new request +++++++\n");
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
