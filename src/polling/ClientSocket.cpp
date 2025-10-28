#include "ClientSocket.hpp"
#include "server.hpp"

//--------------------------- CONSTRUCTORS ----------------------------------//

ClientSocket::ClientSocket(ServerSocket &server)
	: _serv(server), _request(NULL), _isReadingFromPipe(false), _clientState(CLIENT_CONNECTED), _lastEventTime(std::time(NULL))
{
	// #ifdef DEBUG
	// 	std::cerr << "ClientSocket Constructor called" << std::endl;
	// #endif

	int addrlen = sizeof(server.getSocketAddr());

	int socketFd = accept(server.getSocketFd(),
						  (struct sockaddr *)server.getSocketAddr(),
						  (socklen_t *)&addrlen);

	if (socketFd < 0)
	{
		perror("accept() failed with error");
		throw failedSocketAccept();
	}

	setSocketFd(socketFd);

	memset(_buffer, '\0', sizeof _buffer);
}

//--------------------------- DESTRUCTORS -----------------------------------//

ClientSocket::~ClientSocket()
{
	if (_request)
	{
		delete _request;
		_request = NULL;
	}

	if (_response)
	{
		delete _response;
		_response = NULL;
	}

	if (_isReadingFromPipe)
		close(_readingEndOfCgiPipe);

	close(getSocketFd());
}

//------------------------------ SETTER --------------------------------------//

void ClientSocket::resetRequest()
{
	if (_request)
	{
		delete _request;
		_request = NULL;
	}

	if (_response)
	{
		delete _response;
		_response = NULL;
	}

	_status.reset();
}

void ClientSocket::setClientState(e_clientState state)
{
	_clientState = state;
	updateLastEventTime();
}

//------------------------------ GETTER --------------------------------------//

char *ClientSocket::getBuffer()
{
	return (_buffer);
}

Request *ClientSocket::getRequest()
{
	return (_request);
}

ServerSocket &ClientSocket::getServer()
{
	return (_serv);
}

Response *ClientSocket::getResponse()
{
	return (_response);
}

int ClientSocket::getCgiPipeFd()
{
	return (_readingEndOfCgiPipe);
}

e_clientState ClientSocket::getClientState() const
{
	return _clientState;
}

time_t ClientSocket::getLastEventTime() const
{
	return _lastEventTime;
}

bool ClientSocket::hasRequest() const
{
	return (_request != NULL);
}

// also sets as parsed if request has finished, ugly, TO DO should rewrite
bool ClientSocket::hasParsedRequest()
{
	if (!hasRequest())
		return false;

	if (_request->getParsingState() == PARSING_DONE)
		setClientState(CLIENT_HAS_PARSED);

	return (getClientState() == CLIENT_HAS_PARSED);
}

bool ClientSocket::hasFilledResponse() const
{
	if (!hasRequest())
		return false;

	return (getClientState() == CLIENT_HAS_FILLED);
}

bool ClientSocket::hasSentResponse() const
{
	if (!hasRequest())
		return false;

	return (getClientState() == CLIENT_HAS_SENT);
}

bool ClientSocket::isReadingFromPipe() const
{
	return _isReadingFromPipe;
}

//------------------------- MEMBER FUNCTIONS --------------------------------//

void ClientSocket::handleConnection(epoll_event event)
{
	getServer().handleExistingConnection(this, event);
}

void ClientSocket::createNewResponse()
{
	_response = new Response(getRequest());
}

void ClientSocket::deleteResponse()
{
	if (_response)
	{
		delete _response;
		_response = NULL;
	}
}

// adds pipe to epoll to monitor, and read from the pipe
// receiving the response as the content is executed
void ClientSocket::startReadingPipe(int pipeFd)
{
#ifdef DEBUG
	std::cout << "Adding CGI pipe to epoll: " << pipeFd << std::endl;
#endif

	updateLastEventTime();
	_readingEndOfCgiPipe = pipeFd;
	getServer().getEpoll().addPipe(this, pipeFd);
	_isReadingFromPipe = true;
}

void ClientSocket::stopReadingPipe()
{
#ifdef DEBUG
	std::cout << "Removing CGI pipe to epoll" << std::endl;
#endif

	_isReadingFromPipe = false;
	getServer().getEpoll().removePipe(this, _readingEndOfCgiPipe);
	_readingEndOfCgiPipe = -1;
}

void ClientSocket::checkForReadError(int valread)
{
	// if valread is 0 or less, an error has occurred :
	if (valread < 0)
	{
		// no more available data in socket, unlikely if call to epollin
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return;
		// socket reading error
		else
			throw failedSocketRead();
	}
	// socket hung up
	if (valread == 0)
		throw endSocket();
}

void ClientSocket::readRequest()
{
#ifdef DEBUG
	std::cout << "\nClient Socket " << getSocketFd() << " ";
#endif

	updateLastEventTime();

	int valread = recv(getSocketFd(), _buffer, BUFFSIZE, 0);
	checkForReadError(valread);

	// since some data can be interspeced with \0, creating a string of valread size
	std::string requestChunk(_buffer, valread);

	// clear buffer for further use
	memset(_buffer, '\0', sizeof(_buffer));

	// add a Request object if none exist, make it parse the current chunk
	if (_request == NULL)
		_request = new Request(_serv, _status, requestChunk);
	// else add the chunk of unparsed material to the existing request
	else
		_request->addRequestChunk(requestChunk);
}

void ClientSocket::sendResponse()
{
	std::string response = getResponse()->getHTTPResponse();

	size_t totalToSend = response.length();
	size_t totalSent = 0;

	if (getRequest()->hasError())
		std::cout << RED;
	else
		std::cout << GREEN;
	std::cout << " " << getRequest()->getStatus().getStatusCode() << "\n\n" << STOP_COLOR;

	verboseLog("First 100 chars: [" + response.substr(0, 100) + "]");

	while (totalSent < totalToSend)
	{
		ssize_t bytesSent = send(getSocketFd(),
								 response.c_str() + totalSent,
								 totalToSend - totalSent, 0);
		if (bytesSent < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
			{
				response = response.substr(totalSent);
				return;
			}
			perror("send failed");
			return;
		}
		totalSent += bytesSent;
	}

	deleteResponse();

	if (totalSent == totalToSend)
		setClientState(CLIENT_HAS_SENT);

	#ifdef DEBUG
	std::cout << VALID_FORMAT("\n++++++++ Answer has been sent ++++++++ \n");
	#endif
}

void ClientSocket::updateLastEventTime()
{
	_lastEventTime = std::time(NULL);
}

