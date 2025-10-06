#pragma once
# include <iostream>
# include <sys/socket.h>
# include <netinet/in.h>
# include <cstring>
# include <unistd.h>
# include <fcntl.h>
# include <cstdio>


# define MAX_EVENTS 10
# define MAX_SOCKET 10

class Sockette
{

private:

	int					_socketFd;
	sockaddr_in			_socketAddress;		// first element of the sockaddr_in, used in many functions
	int					_socketAddrLen;
	int					_port;

	Sockette(const Sockette &copy);				// should never be used because constructor uses up ressources (ports)
	Sockette &operator=(const Sockette &other);	// should never be used

public:

	//----------------- CONSTRUCTORS ---------------------//

	Sockette();

	//----------------- DESTRUCTOR -----------------------//

	~Sockette();

	//------------------- OPERATORS ----------------------//


	//-------------------- GUETTER -----------------------//

	int					getSocketFd() const;
	const sockaddr_in*	getSocketAddr() const;
	sockaddr_in*		getSocketAddr();
	int					getSocketAddrLen() const;
	int					getPort() const;

	//--------------------- SETTER ------------------------//

	void				setPort(int port);
	void				setSocketFd(int socketFd);

	//--------------- MEMBER FUNCTIONS -------------------//

	void				setSocketOption(int option);
	void				setSocketNonBlocking();
	void				bindToIPAddress();
	void				setListenMode(int maxQueue);
	void				acceptConnectionFrom(Sockette ConnectingSocket);

	//------------------ EXCEPTIONS ----------------------//

	class failedSocketCreation : public std::exception {
		public :
			const char* what() const throw();
	};

	class failedSocketSetOption : public std::exception {
		public :
			const char* what() const throw();
	};

	class failedSocketBinding : public std::exception {
		public :
			const char* what() const throw();
	};

	class failedSocketListen : public std::exception {
		public :
			const char* what() const throw();
	};

	class failedSocketAccept : public std::exception {
		public :
			const char* what() const throw();
	};

	class failedSocketRead : public std::exception {
		public :
			const char* what() const throw();
	};

	class failedFcntl : public std::exception {
		public :
			const char* what() const throw();
	};

	class endSocket : public std::exception {
		public :
			const char* what() const throw();
	};

};
