#include "ClientSocket.hpp"
#include "server.hpp"

//--------------------------- CONSTRUCTORS ----------------------------------//

ClientSocket::ClientSocket(ServerSocket &server)
	: _serv(server)
	, _request(NULL)
{
// #ifdef DEBUG
// 	std::cerr << "ClientSocket Constructor called" << std::endl;
// #endif

	int addrlen = sizeof(server.getSocketAddr());

	int socketFd = accept(server.getSocketFd(), \
		(struct sockaddr *)server.getSocketAddr(), \
		(socklen_t *)&addrlen);

	if (socketFd < 0) {
		perror("accept() failed with error");
		throw failedSocketAccept();
	}

	setSocketFd(socketFd);

	memset(_buffer, '\0', sizeof _buffer);
}

//--------------------------- DESTRUCTORS -----------------------------------//

ClientSocket::~ClientSocket()
{
	delete _request;
	_request = NULL;
	close(getSocketFd());
}

//------------------------------ SETTER --------------------------------------//

void	ClientSocket::resetRequest()
{
	delete _request;
	_request = NULL;
}

//------------------------------ GETTER --------------------------------------//

char*	ClientSocket::getBuffer()
{
	return (_buffer);
}

Request*	ClientSocket::getRequest()
{
	return (_request);
}

ServerSocket&	ClientSocket::getServer()
{
	return (_serv);
}

Response*	ClientSocket::getResponseObject()
{
	return (_responseObject);
}

int	ClientSocket::getCgiPipeFd()
{
	return (_readingEndOfCgiPipe);
}

bool	ClientSocket::hasRequest() const
{
	return (_request != NULL);
}

bool	ClientSocket::hasParsedRequest() const
{
	if (!hasRequest())
		return false;

	return (_request->getParsingState() == PARSING_DONE);
}

bool	ClientSocket::hasFilledResponse() const
{
	if (!hasRequest())
		return false;

	return (_request->getParsingState() == FILLING_DONE);
}

bool	ClientSocket::hasSentResponse() const
{
	if (!hasRequest())
		return false;

	return (_request->getParsingState() == SENDING_DONE);
}

bool	ClientSocket::isReadingFromPipe() const
{
	return _isReadingFromPipe;
}

//------------------------- MEMBER FUNCTIONS --------------------------------//

void	ClientSocket::createNewResponse()
{
	_responseObject = new Response(getRequest());
}

void	ClientSocket::deleteResponse()
{
	delete _responseObject;
}

// adds pipe to epoll to monitor, and read from the pipe
// receiving the response as the content is executed
void	ClientSocket::startReadingPipe(int pipeFd)
{
	#ifdef DEBUG
	std::cout << "Adding CGI pipe to epoll: " << pipeFd << std::endl;
	#endif

	_readingEndOfCgiPipe = pipeFd;
	getServer().getEpoll().addPipe(this, pipeFd);
	_isReadingFromPipe = true;
}

void	ClientSocket::stopReadingPipe()
{
	_isReadingFromPipe = false;
}

void	ClientSocket::checkForReadError(int valread)
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

void	ClientSocket::readRequest()
{
	#ifdef DEBUG
		std::cout << "\nClient Socket " << getSocketFd();
	#endif

	int valread = recv(getSocketFd(), _buffer, BUFFSIZE, 0);
	checkForReadError(valread);

	// since some data can be interspeced with \0, creating a string of valread size
	std::string	requestChunk(_buffer, valread);

	// clear buffer for further use
	memset(_buffer, '\0', sizeof(_buffer));

	// add a Request object if none exist, make it parse the current chunk
	if (_request == NULL)
		_request = new Request(_serv.getConf(), requestChunk);
	// else add the chunk of unparsed material to the existing request
	else
		_request->addRequestChunk(requestChunk);
}

void	ClientSocket::sendResponse()
{


	std::string response = getResponseObject()->getHTTPResponse();

	size_t totalToSend = response.length();
	size_t totalSent = 0;

	std::cout << "Response size: " << response.size() << " bytes" << std::endl;
	std::cout << "First 100 chars: [" << response.substr(0, 100) << "]" << std::endl;

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
	_response.clear();
	std::cout << "Successfully sent " << totalSent << " bytes" << std::endl;
	if (totalSent == totalToSend)
		_request->setParsingState(SENDING_DONE);

	std::cout << VALID_FORMAT("\n++++++++ Answer has been sent ++++++++ \n");
}




