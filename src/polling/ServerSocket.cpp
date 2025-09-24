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

ServerSocket::ServerSocket(ServerConf conf)
	: _conf(conf)
{
	#ifdef DEBUG
		std::cout << "ServerSocket Constructor called" << std::endl;
	#endif

	setPort(conf.getPort());

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


//---------------------------- GETTERS --------------------------------------//

const ServerConf&		ServerSocket::getConf() const
{
	return _conf;
}


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
	ClientSocket*	Connecting = new ClientSocket(*this);

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
	ClientSocket* Connecting = reinterpret_cast<ClientSocket*>(event.data.ptr);

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

void			ServerSocket::listeningLoop()
{
	ContentFetcher	cf;
	cf.addExecutor(new PHPExecutor());
	cf.addExecutor(new PythonExecutor());

	while (1)
	{
		std::cout << "\n\n+++++++ Waiting for new request +++++++\n\n";

		// create a socket to receive incoming communication
		ClientSocket AnsweringSocket(*this);
		try {
			// reading the request into the Sockette buffer
			AnsweringSocket.readRequest();

			// decoding the buffer into a Request object
			Request decodedRequest(_conf, AnsweringSocket.getRequest());

			// creating a Response handling request according to configured routes
			Response response(_conf, decodedRequest);

			cf.fillContent(response);

			write(AnsweringSocket.getSocketFd(), response.getHTTPResponse().c_str(), response.getHTTPResponse().size());

			std::cout << "\n\n+++++++ Answer has been sent +++++++ \n\n";
		}

		catch ( HTTPError &e )
		{
			std::cout << ERROR_FORMAT("\n\n+++++++ HTTP Error Page +++++++ \n\n");
			std::cout << "response is [" << e.getErrorPage() << "\n";
			write(AnsweringSocket.getSocketFd(), e.getErrorPage().c_str(), e.getErrorPage().size());
			std::cerr << e.what() << "\n";
		}

		catch ( std::exception &e )
		{
			std::cout << ERROR_FORMAT("\n\n+++++++ Non HTTP Error +++++++ \n\n");
			std::cerr << e.what() << "\n";
		}
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
