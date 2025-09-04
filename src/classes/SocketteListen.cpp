# include "SocketteListen.hpp"

//--------------------------- CONSTRUCTORS ----------------------------------//

SocketteListen::SocketteListen(int port)
{
	#ifdef DEBUG
		std::cout << "SocketteListen Constructor called" << std::endl;
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
}

//--------------------------- DESTRUCTORS -----------------------------------//



//---------------------------- OPERATORS ------------------------------------//

