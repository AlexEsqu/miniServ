#pragma once
# include <iostream>
# include <sys/socket.h>
# include <netinet/in.h>

class Sockette
{

private:

	int				_socketFd;
	sockaddr_in		_socketAddress;
	int				_socketAddrLen;

public:

	//----------------- CONSTRUCTORS ---------------------//

	Sockette();
	Sockette(const Sockette &copy);

	//----------------- DESTRUCTOR -----------------------//

	~Sockette();

	//------------------- OPERATORS ----------------------//

	Sockette &operator=(const Sockette &other);

	//-------------------- GUETTER -----------------------//

	int				getSocketFd();
	sockaddr_in		getSocketAddr();
	int				getSocketAddrLen();

	//--------------- MEMBER FUNCTIONS -------------------//

	void			setSocketOption(int option);

	//------------------ EXCEPTIONS ----------------------//

	class failedSocketCreation : public std::exception {
		public :
			const char* what() const throw();
	};

	class failedSocketSetOption : public std::exception {
		public :
			const char* what() const throw();
	};

};
