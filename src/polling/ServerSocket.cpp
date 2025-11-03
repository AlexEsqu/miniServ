# include "ServerSocket.hpp"

//--------------------------- CONSTRUCTORS ----------------------------------//

ServerSocket::ServerSocket(Poller& poller, ServerConf& conf)
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

	//fcntl(getSocketFd(), F_SETFL, O_NONBLOCK);
	Sockette::setFdAsClosingOnExecution(getSocketFd());

	_poller.addServerSocket(*this);
}

//--------------------------- DESTRUCTORS -----------------------------------//

ServerSocket::~ServerSocket()
{
	while (!_clients.empty()) {
		removeConnection(_clients.begin()->second);
	}

	if (getSocketFd() >= 0)
		_poller.removeSocket(this);

	delete _cf;
}

//---------------------------- OPERATORS ------------------------------------//


//---------------------------- GETTERS --------------------------------------//

ServerConf&		ServerSocket::getConf()
{
	return _conf;
}

Poller&					ServerSocket::getEpoll()
{
	return _poller;
}

std::map<size_t, Session>&	ServerSocket::getSessionMap()
{
	return _sessionMap;
}

//------------------------ MEMBER FUNCTIONS ---------------------------------//

void			ServerSocket::handleConnection(epoll_event)
{
	acceptNewConnection();
}

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

	setFdAsClosingOnExecution(Connecting->getSocketFd());

	#ifdef DEBUG
		std::cout << CONNEX_FORMAT("\n++++ Accepted Connection on Socket ";
		std::cout  << Connecting->getSocketFd() << " ++++ \n");
	#endif
}

void			ServerSocket::removeConnection(ClientSocket* clientSocket)
{
	#ifdef DEBUG
		std::cout << "\n++++ Removing Client Socket ";
		std::cout  << clientSocket->getSocketFd() << " ++++ \n";
	#endif

	if (_clients.find(clientSocket->getSocketFd()) != _clients.end())
	{
		_clients.erase(clientSocket->getSocketFd());
		_poller.removeSocket(clientSocket);
		delete clientSocket;
	}
}

// Client generally receives data from web user to parse into a request
// but may also read from an internal pipe CGI being executed for a response
void			ServerSocket::receiveAndParseData(ClientSocket* client)
{
	if (client->hasTimedOut())
		return ;

	// special case when the CGI is being executed, requires specific functions
	if (client->isReadingFromPipe())
		_cf->readCGIChunk(client);

	// normal case : reading the request or request chunk into a Request object
	else
	{
		client->readRequest();

		if (client->hasParsedRequest())
			_cf->fillResponse(client);
		if (client->hasFilledResponse())
			_poller.setPollingMode(WRITING, client);
	}
}

void			ServerSocket::sendDataIfComplete(ClientSocket* client)
{
	if (client->hasFilledResponse())
	{
		client->updateLastEventTime();

		client->sendResponse();

		if (client->hasSentResponse())
			closeConnectionOrCleanAndKeepAlive(client);
	}
}

void			ServerSocket::handleExistingConnection(ClientSocket* client, epoll_event &event)
{
	try
	{
		if (socketIsReadyToReceiveData(event))
		{
			receiveAndParseData(client);
		}
		else if (socketIsReadyToSendData(event))
		{
			sendDataIfComplete(client);
		}
		else if (socketIsHangingUp(event))
		{
			removeConnection(client);
		}
		else if (socketIsHavingTrouble(event))
		{
			throw std::runtime_error("Connection closed or error occurred");
		}
	}

	// if a soket hangs up, close it, it has ended its purpose
	catch (endSocket& e)
	{
		removeConnection(client);
	}

	// catch system error such as alloc, read, or write fail, and remove faulty sockets
	catch ( std::exception &e )
	{
		std::cout << ERROR_FORMAT("\n\n+++++++ Non HTTP Error +++++++ \n") << e.what() << "\n";
		removeConnection(client);
	}
}

void		ServerSocket::closeConnectionOrCleanAndKeepAlive(ClientSocket* socket)
{
	if (socket->getRequest().isKeepAlive() && !socket->hasTimedOut())
	{
		socket->resetContent();
		socket->updateLastEventTime();
		_poller.setPollingMode(READING, socket);
	}
	else
		removeConnection(socket);
}

void		ServerSocket::timeoutRequest(ClientSocket& client)
{
	client.getCgiBuffer().clearBuffer();
	client.getResponse().reset();
	client.getRequest().setError(REQUEST_TIMEOUT);
	client.setTimedOut(true);
	client.getRequest().setKeepAlive(false);
	client.getResponse().createHTTPHeaders();
	client.setClientState(CLIENT_HAS_FILLED);
	if (client.isReadingFromPipe())
		client.stopReadingPipe();
	else
		_poller.setPollingMode(WRITING, &client);
	if (client.getRequest().getCgiForkPid() > 0)
	{
		if (kill(client.getRequest().getCgiForkPid(), SIGKILL) == -1)
			perror("kill");
		client.getRequest().setCgiForkPid(0);
	}
}

void		ServerSocket::timeoutCgi(ClientSocket& client)
{
	client.getCgiBuffer().clearBuffer();
	client.getResponse().reset();
	client.getRequest().setError(BAD_GATEWAY);
	client.setTimedOut(true);
	client.getRequest().setKeepAlive(false);
	client.getResponse().createHTTPHeaders();
	client.setClientState(CLIENT_HAS_FILLED);
	if (client.isReadingFromPipe())
		client.stopReadingPipe();
	else
		_poller.setPollingMode(WRITING, &client);
	if (client.getRequest().getCgiForkPid() > 0)
	{
		if (kill(client.getRequest().getCgiForkPid(), SIGKILL) == -1)
			perror("kill");
		client.getRequest().setCgiForkPid(0);
	}
}

// checks all client sockets, remove the one who did not set off any events in a while
void		ServerSocket::timeoutIdleClients()
{
	if (_clients.empty())
		return;

	time_t						currentTime = std::time(NULL);
	std::vector<ClientSocket*>	clientsToRemove;

	for (std::map<int, ClientSocket*>::iterator it = _clients.begin();
		it != _clients.end(); ++it)
	{
		ClientSocket*	client = it->second;
		time_t			idleTime = currentTime - client->getLastEventTime();

		// if the client is reading from a pipe, it is awaiting a CGI execution which may be timeout
		// but with a BAD_GETAWAY error to signify the CGI cannot be executed and need not be resent
		if (client->isReadingFromPipe())
		{
			time_t			cgiTime = currentTime - client->getRequest().getCgiStartTime();
			if (cgiTime > TIMEOUT_CGI)
				timeoutCgi(*client);
		}

		// otherwise it may be stuck for other reasons and need to be put down with a REQUEST TIMEOUT
		else if (idleTime > TIMEOUT_CONNECTION)
			timeoutRequest(*client);
	}
}

// socket is ready to receive data or hanging up (recv 0 byte)
bool		ServerSocket::socketIsReadyToReceiveData(epoll_event& event)
{
	return (event.events & EPOLLIN || socketIsHangingUp(event));
}

bool		ServerSocket::socketIsReadyToSendData(epoll_event& event)
{
	return (event.events & EPOLLOUT);
}

bool		ServerSocket::socketIsHangingUp(epoll_event& event)
{
	return (event.events & (EPOLLHUP | EPOLLRDHUP));
}

bool		ServerSocket::socketIsHavingTrouble(epoll_event& event)
{
	return (event.events & EPOLLERR);
}
