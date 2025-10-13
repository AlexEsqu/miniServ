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

void	ClientSocket::setResponse(std::string response)
{
	_response = response;
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

std::string&	ClientSocket::getResponse()
{
	return (_response);
}

//------------------------- MEMBER FUNCTIONS --------------------------------//

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

// read MAX_HEADER_SIZE into socket to check if request has the substring
// "\r\n\r\b" which means the request headers have been received in full
bool	ClientSocket::tryToReadHeaderBlock()
{
	// if a request object exists, the headers have already been extracted and parsed
	if (_request)
		return true;

	int valread = recv(getSocketFd(), _buffer, MAX_HEADER_SIZE, O_NONBLOCK);
	checkForReadError(valread);

	// for ease of string manipulation, get buffer into a string and clear buffer
	std::string	headersAsString(_buffer, valread);
	memset(_buffer, '\0', MAX_HEADER_SIZE);

	// searchs for the end of header string in the read from the buffer
	size_t		posEndOfHeader = headersAsString.find(END_OF_HEADER_STR);

	// if the end of header is not in the socket, the request headers have not all been received
	if (posEndOfHeader == std::string::npos)
	{
		// much like NGINX or Apache, max length of headers is set at 4k
		// so if no end of header is found in that amount, throw HTTP Error 413 Entity Too Large
		if (valread == MAX_HEADER_SIZE)
			throw HTTPError(NULL, 413); // TO DO: add a request there or change constructor to allow for request less

		// else if the socket simply doesn't contain the MAX_HEADER_SIZE yet,
		// simply return false and waits for more
		return false;
	}

	// else sets headerSize and return true
	_headerSize = posEndOfHeader + sizeof(END_OF_HEADER_STR);
	_fullHeader = headersAsString;
	return true;
}

void	ClientSocket::readRequest()
{
	#ifdef DEBUG
		std::cout << "\nClient Socket " << getSocketFd();
	#endif

	// check if the full request headers have been received to extract them, else return to epoll
	if (!tryToReadHeaderBlock())
		return;

	// add a Request object if none exist, make it parse the headers and possible leftover
	if (_request == NULL)
		_request = new Request(_serv.getConf(), _fullHeader);

	// if the request needs more than headers, read into a buffer and add as chunk
	else if (_request->getParsingState() == PARSING_BODY || _request->getParsingState() == PARSING_BODY_CHUNKED)
	{
		int valread = recv(getSocketFd(), _buffer, BUFFSIZE, O_NONBLOCK); // TO DO : optimize to call for content length if known ?
		checkForReadError(valread);

		// since some data can be interspeced with \0, creating a string of valread size
		std::string	requestChunk(_buffer, valread);

		_request->addRequestChunk(_buffer);

		// clear buffer for further use
		memset(_buffer, '\0', sizeof(_buffer));
	}

}

void ClientSocket::sendResponse()
{
	if (_response.empty())
		return;

	size_t totalToSend = _response.length();
	size_t totalSent = 0;

	std::cout << "Response size: " << _response.size() << " bytes" << std::endl;
	std::cout << "First 100 chars: [" << _response.substr(0, 100) << "]" << std::endl;

	while (totalSent < totalToSend)
	{
		ssize_t bytesSent = send(getSocketFd(),
								_response.c_str() + totalSent,
								totalToSend - totalSent, 0);

		if (bytesSent < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
			{
				_response = _response.substr(totalSent);
				return;
			}
			perror("send failed");
			return;
		}

		totalSent += bytesSent;
	}
	_response.clear();
	std::cout << "Successfully sent " << totalSent << " bytes" << std::endl;

	std::cout << VALID_FORMAT("\n++++++++ Answer has been sent ++++++++ \n");
}

