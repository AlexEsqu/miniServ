#include "ClientSocket.hpp"
#include "server.hpp"

//--------------------------- CONSTRUCTORS ----------------------------------//

ClientSocket::ClientSocket(ServerSocket &server)
	: _serv(server)
	, _request(Request(server.getConf(), _status))
	, _response(Response(_request, _status))
	, _isReadingFromPipe(false)
	, _clientState(CLIENT_CONNECTED)
	, _lastEventTime(std::time(NULL))
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
	if (_isReadingFromPipe)
		close(_readingEndOfCgiPipe);

	close(getSocketFd());
}

//------------------------------ SETTER --------------------------------------//

void ClientSocket::resetContent()
{
	_request.reset();

	_response.reset();

	_status.reset();

	_clientState = CLIENT_CONNECTED;
}

void ClientSocket::setClientState(e_clientState state)
{
	_clientState = state;
	updateLastEventTime();
}

//------------------------------ GETTER --------------------------------------//

Request&		ClientSocket::getRequest()
{
	return (_request);
}

ServerSocket&	ClientSocket::getServer()
{
	return (_serv);
}

Response&		ClientSocket::getResponse()
{
	return (_response);
}

Status&			ClientSocket::getStatus()
{
	return (_status);
}


char*			ClientSocket::getBuffer()
{
	return (_buffer);
}

int				ClientSocket::getCgiPipeFd()
{
	return (_readingEndOfCgiPipe);
}


e_clientState	ClientSocket::getClientState() const
{
	return _clientState;
}

time_t			ClientSocket::getLastEventTime() const
{
	return _lastEventTime;
}

bool			ClientSocket::hasStartedRequest() const
{
	return (_request.getParsingState() != EMPTY);
}

// also sets as parsed if request has finished, ugly, TO DO should rewrite
bool			ClientSocket::hasParsedRequest()
{
	if (!hasStartedRequest())
		return false;

	if (_request.getParsingState() == PARSING_DONE)
		setClientState(CLIENT_HAS_PARSED);

	return (getClientState() == CLIENT_HAS_PARSED);
}

bool			ClientSocket::hasFilledResponse() const
{
	if (!hasStartedRequest())
		return false;

	return (getClientState() == CLIENT_HAS_FILLED);
}

bool			ClientSocket::hasSentResponse() const
{
	if (!hasStartedRequest())
		return false;

	return (getClientState() == CLIENT_HAS_SENT);
}

bool			ClientSocket::isReadingFromPipe() const
{
	return _isReadingFromPipe;
}

//------------------------- MEMBER FUNCTIONS --------------------------------//

void			ClientSocket::handleConnection(epoll_event event)
{
	getServer().handleExistingConnection(this, event);
}

// adds pipe to epoll to monitor, and read from the pipe
// receiving the response as the content is executed
void			ClientSocket::startReadingPipe(int pipeFd)
{
#ifdef DEBUG
	std::cout << "Adding CGI pipe to epoll: " << pipeFd << std::endl;
#endif

	updateLastEventTime();
	_readingEndOfCgiPipe = pipeFd;
	getServer().getEpoll().addPipe(this, pipeFd);
	_isReadingFromPipe = true;
}

void			ClientSocket::stopReadingPipe()
{
#ifdef DEBUG
	std::cout << "Removing CGI pipe to epoll" << std::endl;
#endif

	_isReadingFromPipe = false;
	getServer().getEpoll().removePipe(this, _readingEndOfCgiPipe);
	_readingEndOfCgiPipe = -1;
}

void			ClientSocket::checkForReadError(int valread)
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

void			ClientSocket::readRequest()
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

	_request.addRequestChunk(requestChunk);
}

void			ClientSocket::sendResponse()
{
	std::string response = getResponse().getHTTPResponse();

	size_t totalToSend = response.length();
	size_t totalSent = 0;

	if (getRequest().hasError())
		std::cout << RED;
	else
		std::cout << GREEN;
	std::cout << " " << getStatus().getStatusCode() << "\n\n" << STOP_COLOR;

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

	if (totalSent == totalToSend)
	{
		setClientState(CLIENT_HAS_SENT);
	}

	#ifdef DEBUG
	std::cout << VALID_FORMAT("\n++++++++ Answer has been sent ++++++++ \n");
	#endif
}

void			ClientSocket::updateLastEventTime()
{
	_lastEventTime = std::time(NULL);
}

size_t			ClientSocket::generateRandomNumber()
{
	std::ifstream file("/dev/random", std::ios_base::in | std::ios_base::binary);
	if (!file)
	{
		std::cerr << "Failed to open /dev/random" << std::endl;
		return 1;
	}

	// Create a stream buffer to read from the file
	std::streambuf *buffer = file.rdbuf();

	// Generate a random number from the read data
	size_t randomNumber = 0;
	char ch;
	while (buffer->sgetn(&ch, 1) > 0)
	{
		randomNumber = (randomNumber << 8) + ch;
	}
	return (randomNumber);
}
