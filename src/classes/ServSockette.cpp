# include "ServSockette.hpp"

//--------------------------- CONSTRUCTORS ----------------------------------//

ServSockette::ServSockette(int port)
{
	#ifdef DEBUG
		std::cout << "ServSockette Constructor called" << std::endl;
	#endif

	setPort(port);

	// allow socket to be reused and webserv to reload faster with SO_REUSEADDR
	setSocketOption(SO_REUSEADDR);

	// bind serv socket to IP address using the standard IP options and provided port
	bindToIPAddress();

	// set serv sock to listen for incomming connections with max incomming
	setListenMode(10);
}

//--------------------------- DESTRUCTORS -----------------------------------//

ServSockette::~ServSockette()
{
	#ifdef DEBUG
		std::cout << "ServSockette Destructor called" << std::endl;
	#endif
}

//---------------------------- OPERATORS ------------------------------------//

