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
	epoll_ctl(_serv.getEpoll(), EPOLL_CTL_DEL, getSocketFd(), NULL);
	close(getSocketFd());
}

//------------------------------ SETTER --------------------------------------//

void	ClientSocket::setEvent(uint32_t epollEventMask)
{
	_event.events = epollEventMask;
	// adding new socket pointer as context in the event itself
	_event.data.ptr = this;
}

void	ClientSocket::resetRequest()
{
	delete _request;
	_request = NULL;
}

void	ClientSocket::setResponse(Response response)
{
	_response = response;
}

//------------------------------ GETTER --------------------------------------//

char*	ClientSocket::getBuffer()
{
	return (_buffer);
}

struct epoll_event&	ClientSocket::getEvent()
{
	return (_event);
}

Request*	ClientSocket::getRequest()
{
	return (_request);
}

ServerSocket&	ClientSocket::getServer()
{
	return (_serv);
}

Response&	ClientSocket::getResponse()
{
	return (_response);
}

//------------------------- MEMBER FUNCTIONS --------------------------------//

void	ClientSocket::readRequest()
{
	#ifdef DEBUG
		std::cout << "\nClient Socket " << getSocketFd() << " rcv";
	#endif

	// read the Client's request into a buffer
	int valread = recv(getSocketFd(), _buffer, BUFFSIZE, O_NONBLOCK);
	if (valread < 0)
		throw failedSocketRead();
	if (valread == 0)
		throw endSocket();

	// add buffer content to a Request object
	if (_request == NULL)
		_request = new Request(_serv.getConf(), _buffer);
	else
		_request->addRequestChunk(_buffer);

	// clear buffer for further use
	memset(_buffer, '\0', sizeof _buffer);
}

void	ClientSocket::sendResponse()
{
	#ifdef DEBUG
		std::cout << _response.getStatus() << std::endl;
	#endif

	if (write(getSocketFd(),
		getResponse().getHTTPResponse().c_str(),
		getResponse().getHTTPResponse().size()) < 0)
	{
		perror("write");
		throw std::runtime_error("write fail");
	}

	std::cout << VALID_FORMAT("\n++++++++ Answer has been sent ++++++++ \n");
}


//    A process for decoding the chunked transfer coding can be represented
//    in pseudo-code as:

//      length := 0
//      read chunk-size, chunk-ext (if any), and CRLF
//      while (chunk-size > 0) {
//         read chunk-data and CRLF
//         append chunk-data to decoded-body
//         length := length + chunk-size
//         read chunk-size, chunk-ext (if any), and CRLF
//      }
//      read trailer field
//      while (trailer field is not empty) {
//         if (trailer field is allowed to be sent in a trailer) {
//             append trailer field to existing header fields
//         }
//         read trailer-field
//      }
//      Content-Length := length
//      Remove "chunked" from Transfer-Encoding
//      Remove Trailer from existing header fields

// void readRequestHeader
// read request header into buffer
// to be then used by Request Object and decoded, so we have content length for further reading OR throwing out error cuz bad request


// once we have content length from header, we can read the body into a string (if it can hold enough ???)
