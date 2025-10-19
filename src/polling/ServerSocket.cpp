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

void			ServerSocket::handleExistingConnection(ClientSocket* client, epoll_event &event)
{
	if (socketIsReadyToReceiveData(event))
	{
		// reading the request or request chunk into a Request object
		try
		{
			if (client->isReadingFromPipe())
			{
				_cf->readCGIChunk(client);
			}
			else
			{
				client->readRequest();

				if (client->hasParsedRequest())
					_cf->fillResponse(client);

				if (client->hasFilledResponse())
				{
					std::cout << "response is filled\n";
					_poller.setPollingMode(WRITING, client);

				}
			}
		}

		// catch system error such as alloc, read, or write fail, and remove faulty sockets
		catch ( std::exception &e )
		{
			std::cout << ERROR_FORMAT("\n\n+++++++ Non HTTP Error +++++++ \n") << e.what() << "\n";
			removeConnection(client);
		}
	}

	else if (socketIsReadyToSendData(event))
	{
		std::cout << "socket " << client->getSocketFd() << "is ready to write\n";

		if (client->hasFilledResponse())
		{
			client->sendResponse();

			if (client->hasSentResponse())
				closeConnectionOrCleanAndKeepAlive(client);
		}
	}

	else if (socketIsHavingTrouble(event)) {
		std::cout << "Connection closed or error occurred" << std::endl;
		removeConnection(client);
		return;
	}
}

void		ServerSocket::closeConnectionOrCleanAndKeepAlive(ClientSocket* socket)
{
	if (socket->getRequest()->isKeepAlive())
	{
		socket->resetRequest();
		_poller.setPollingMode(READING, socket);
	}
	else
		removeConnection(socket);
}

bool		ServerSocket::socketIsReadyToReceiveData(epoll_event& event)
{
	return (event.events & (EPOLLIN | EPOLLHUP));
}

bool		ServerSocket::socketIsReadyToSendData(epoll_event& event)
{
	return (event.events & EPOLLOUT);
}

bool		ServerSocket::socketIsHavingTrouble(epoll_event& event)
{
	return (event.events & (EPOLLERR | EPOLLRDHUP));
}
