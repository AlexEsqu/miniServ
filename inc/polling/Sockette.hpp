#pragma once
# include <iostream>
# include <sys/socket.h>
# include <sys/epoll.h>
# include <netinet/in.h>
# include <cstring>
# include <unistd.h>
# include <fcntl.h>
# include <cstdio>

class ClientSocket;

# define MAX_EVENTS 10
# define MAX_SOCKET 10

class Sockette
{

private:

	int					_socketFd;
	sockaddr_in			_socketAddress;		// first element of the sockaddr_in, used in many functions
	int					_socketAddrLen;
	int					_port;
	struct epoll_event	_epollEvent;

	Sockette(const Sockette &copy);				// should never be used because constructor uses up ressources (ports)
	Sockette &operator=(const Sockette &other);	// should never be used

public:

	//----------------- CONSTRUCTORS ---------------------//

	Sockette();

	//----------------- DESTRUCTOR -----------------------//

	virtual ~Sockette();

	//------------------- OPERATORS ----------------------//


	//-------------------- GUETTER -----------------------//

	int					getSocketFd() const;
	const sockaddr_in*	getSocketAddr() const;
	sockaddr_in*		getSocketAddr();
	int					getSocketAddrLen() const;
	int					getPort() const;
	struct epoll_event&	getEpollEventsMask();

	//--------------------- SETTER ------------------------//

	void				setPort(int port);
	void				setSocketFd(int socketFd);
	void				setEpollEventsMask(uint32_t epollEventMask);

	//--------------- MEMBER FUNCTIONS -------------------//

	void				setSocketOption(int option);
	void				setSocketNonBlocking();
	void				bindToIPAddress();
	void				setListenMode(int maxQueue);
	void				acceptConnectionFrom(ClientSocket* client);
	virtual void		handleConnection(epoll_event event) = 0;

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
