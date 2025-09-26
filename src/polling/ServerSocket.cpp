# include "ServerSocket.hpp"

//--------------------------- CONSTRUCTORS ----------------------------------//

ServerSocket::ServerSocket(int port) // TO BE DESTROYED
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

	createEpollInstance();
}

ServerSocket::ServerSocket(ServerConf conf)
	: _conf(conf)
{
	#ifdef DEBUG
		std::cout << "ServerSocket Constructor called" << std::endl;
	#endif

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

		// Remove from epoll
		epoll_ctl(_epollFd, EPOLL_CTL_DEL, Connecting->getSocketFd(), NULL);

		// Clean up memory
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
		perror("Failed to add server socket to epoll");
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
	Connecting->setEvent(EPOLLIN | EPOLLOUT | EPOLLET);

	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, Connecting->getSocketFd(), &Connecting->getEvent()) == -1) {
		throw failedEpollCtl();
	}

	#ifdef DEBUG
		std::cout << "Connection established" << std::endl;
		std::cout << "Fd is " << Connecting->getSocketFd() << std::endl;
	#endif

}

void			ServerSocket::handleExistingConnection(epoll_event &event)
{
	ClientSocket* Connecting = reinterpret_cast<ClientSocket*>(event.data.ptr);

	// Socket ready to read
	if (event.events & EPOLLIN) {

		try {

			// reading the request into the Sockette buffer
			Connecting->readRequest();

			if (Connecting->getRequest()->isComplete())
			{
				// creating a Response handling request according to configured routes
				Response response(Connecting->getRequest());

				_cf->fillContent(response);

				write(Connecting->getSocketFd(), response.getHTTPResponse().c_str(), response.getHTTPResponse().size());

				std::cout << "\n\n+++++++ Answer has been sent +++++++ \n\n";

			}
			else
			{

			}

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
		}
	}

	if (event.events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
		std::cout << "Connection closed or error occurred" << std::endl;

		// Remove from epoll
		epoll_ctl(_epollFd, EPOLL_CTL_DEL, Connecting->getSocketFd(), NULL);

		// Clean up memory
		delete Connecting;
		return;
	}

	#ifdef DEBUG
		std::cout << "Connection handled" << std::endl;
	#endif

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

// void			ServerSocket::listeningLoop()
// {
// 	ContentFetcher	cf;
// 	cf.addExecutor(new PHPExecutor());
// 	cf.addExecutor(new PythonExecutor());

// 	while (1)
// 	{
// 		std::cout << "\n\n+++++++ Waiting for new request +++++++\n\n";

// 		// create a socket to receive incoming communication
// 		ClientSocket AnsweringSocket(*this);
// 		try {
// 			// reading the request into the Sockette buffer
// 			AnsweringSocket.readRequest();

// 			// decoding the buffer into a Request object
// 			Request decodedRequest(_conf, AnsweringSocket.getRequest());

// 			// creating a Response handling request according to configured routes
// 			Response response(_conf, decodedRequest);

// 			cf.fillContent(response);

// 			write(AnsweringSocket.getSocketFd(), response.getHTTPResponse().c_str(), response.getHTTPResponse().size());

// 			std::cout << "\n\n+++++++ Answer has been sent +++++++ \n\n";
// 		}

// 		catch ( HTTPError &e )
// 		{
// 			std::cout << ERROR_FORMAT("\n\n+++++++ HTTP Error Page +++++++ \n\n");
// 			std::cout << "response is [" << e.getErrorPage() << "\n";
// 			write(AnsweringSocket.getSocketFd(), e.getErrorPage().c_str(), e.getErrorPage().size());
// 			std::cerr << e.what() << "\n";
// 		}

// 		catch ( std::exception &e )
// 		{
// 			std::cout << ERROR_FORMAT("\n\n+++++++ Non HTTP Error +++++++ \n\n");
// 			std::cerr << e.what() << "\n";
// 		}
// 	}
// }

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
