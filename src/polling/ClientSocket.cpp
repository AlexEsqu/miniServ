#include "ClientSocket.hpp"
#include "server.hpp"

//--------------------------- CONSTRUCTORS ----------------------------------//

ClientSocket::ClientSocket(ServerSocket &server)
{
#ifdef DEBUG
	std::cout << "ClientSocket Constructor called" << std::endl;
#endif

	int addrlen = sizeof(server.getSocketAddr());

	int socketFd = accept(server.getSocketFd(), \
		(struct sockaddr *)server.getSocketAddr(), \
		(socklen_t *)&addrlen);

	if (socketFd < 0) {
		perror("accept() failed with error");
		throw failedSocketAccept();
	}

	setSocketFd(socketFd);

	server.addSocketToEpoll(*this);

	memset(_buffer, '\0', sizeof _buffer);
}

//--------------------------- DESTRUCTORS -----------------------------------//

//------------------------------ GETTER --------------------------------------//

char *ClientSocket::getRequest()
{
	return (_buffer);
}

//------------------------- MEMBER FUNCTIONS --------------------------------//

void ClientSocket::readRequest()
{
	// readRequestHeader();
	int valread = read(getSocketFd(), _buffer, BUFFSIZE);

	if (valread < 0)
		throw failedSocketRead();

	// #ifdef DEBUG
	// 	std::cout << "Answer socket read " << valread << " bytes: [" << _buffer << "]\n" << std::endl;
	// #endif
}

std::string tolower(std::string str)
{
	for (size_t i = 0; i < str.length(); i++)
		str[i] = tolower(str[i]);
	return (str);
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

void ClientSocket::readRequestHeader()
{
	int valread = -1;
	std::string line;
	bool isHeader = true;

	valread = read(getSocketFd(), _buffer, BUFFSIZE);
	if (valread < 0)
		throw failedSocketRead();
	if (valread == 0)
		return;
	std::istringstream buffer(_buffer);
	while (std::getline(buffer, line) && isHeader)
	{
		_header += line + '\n';
		if (line == "\r")
		{
			isHeader = false;
			break;
		}
	}
	std::cout << MAGENTA << _header << STOP_COLOR << std::endl;
	if (_contentLength > 0)
		readRequestBody(buffer);
}

// void readRequestHeader
// read request header into buffer
// to be then used by Request Object and decoded, so we have content length for further reading OR throwing out error cuz bad request

std::string ClientSocket::readRequestBody(std::istringstream &buffer)
{
	std::string line;
	while (std::getline(buffer, line))
	{
		_body += line + '\n';
	}
	std::cerr << GREEN << _body << STOP_COLOR << std::endl;
	return (_body);
}
// once we have content length from header, we can read the body into a string (if it can hold enough ???)
