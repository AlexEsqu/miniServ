# include "ServerSocket.hpp"

//--------------------------- CONSTRUCTORS ----------------------------------//

ServerSocket::ServerSocket(Poller& poller, const ServerConf& conf)
	: _poller(poller)
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

	// fcntl(getSocketFd(), F_SETFL, O_NONBLOCK);

	_poller.addServerSocket(*this);
}

//--------------------------- DESTRUCTORS -----------------------------------//

ServerSocket::~ServerSocket()
{
	while (!_clients.empty()) {
		removeConnection(_clients.begin()->second);
	}

	_poller.removeSocket(this);

	delete _cf;
}

//---------------------------- OPERATORS ------------------------------------//


//---------------------------- GETTERS --------------------------------------//

const ServerConf&		ServerSocket::getConf() const
{
	return _conf;
}

Poller&					ServerSocket::getEpoll()
{
	return _poller;
}

//------------------------ MEMBER FUNCTIONS ---------------------------------//

void			ServerSocket::acceptNewConnection()
{
	// allocating new acccepting socket to be used
	ClientSocket*	Connecting = new ClientSocket(*this);

	Connecting->setSocketNonBlocking();
	Connecting->setEpollEventsMask(EPOLLIN | EPOLLERR);
	try
	{
		_poller.addSocket(*Connecting);
	}
	catch (std::exception& e)
	{
		delete Connecting;
		throw std::runtime_error("Epoll ctl() failed");
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

	_poller.removeSocket(clientSocket);

	delete clientSocket;
}

void			ServerSocket::handleExistingConnection(ClientSocket* connecting, epoll_event &event)
{
	// Socket ready to read data
	if (event.events & EPOLLIN) {

		// reading the request or request chunk into a Request object
		try
		{
			connecting->readRequest();

			if (connecting->getRequest() && connecting->getRequest()->getParsingState() == PARSING_DONE)
			{
				_cf->fillRequest(*(connecting->getRequest()));
				// TO DO : First execute CGI / get page, only once fully gotten can create response
				connecting->setResponse(connecting->getRequest()->getResponse()->getHTTPResponse());
				connecting->setEpollEventsMask(EPOLLOUT | EPOLLERR);
				_poller.updateSocketEvent(connecting);
			}
		}

		catch ( HTTPError &e )
		{
			std::cout << ERROR_FORMAT("\n\n+++++++ HTTP Error Page +++++++ \n\n");
			std::cerr << e.what() << "\n";
			connecting->setEpollEventsMask(EPOLLOUT | EPOLLERR);
			_poller.updateSocketEvent(connecting);
		}

		catch ( std::exception &e )
		{
			std::cout << ERROR_FORMAT("\n\n+++++++ Non HTTP Error +++++++ \n\n");
			std::cerr << e.what() << "\n";
			removeConnection(connecting);
		}
	}

	// socket ready to receive data
	else if (event.events & EPOLLOUT)
	{
		// if request is complete, fetch content and wrap in HTTP header
		if (connecting->getRequest() &&
			(connecting->getRequest()->getParsingState() == PARSING_DONE
				|| connecting->getRequest()->getStatus().getStatusCode() >= 400))
		{
			connecting->sendResponse();

			if (connecting->getRequest()->isKeepAlive())
			{
				connecting->resetRequest();
				connecting->setEpollEventsMask(EPOLLIN | EPOLLERR);
				_poller.updateSocketEvent(connecting);
			}
			else
				removeConnection(connecting);
		}
	}

	// Socket not doing so well
	else if (event.events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
		std::cout << "Connection closed or error occurred" << std::endl;
		removeConnection(connecting);
		return;
	}
}




