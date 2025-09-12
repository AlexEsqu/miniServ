#pragma once
#include <iostream>
#include <string>
#include <algorithm>
#include <vector>

#include "readability.hpp"

class Request;

class Request
{

private:

	//------------------ ATTRIBUTES ----------------------//

	std::string _fullRequest;	// full content of the request
	std::string _method;		// could be set as the enum already ?
	std::string _protocol;		// probably not needed, unless we only support HTTP/1.1
	std::string _host;			// for example "example.com"
	std::string _connection;	// to research keep-alive
	std::string _requestedFileName;	// for example "/home.html"
	std::string _contentType;
	int			_CGI;			// 0 NO CGI, 1 PY, 2 PHP

	//-------------- INTERNAL FUNCTIONS -------------------//

	std::string extractMethodFromHTTP(std::string::iterator &it);
	std::string extractProtocolFromHTTP(std::string::iterator &it);
	std::string extractURLFromHTTP(std::string::iterator &it);
	void		fillEnvFromHTTPHeader(std::string &httpRequest, std::string::iterator &curr);
	void		checkHTTPValidity(std::string &htmlRequest, std::string::iterator &it);
	std::vector<std::string>	_requestEnv;	// all header variables as ENV in a vector

public:

	//----------------- CONSTRUCTORS ---------------------//

	Request(std::string httpRequest);
	Request(const Request &copy);

	//----------------- DESTRUCTOR -----------------------//

	~Request();

	//-------------------- GETTERS -----------------------//

	std::string		getMethod() const;
	std::string		getProtocol() const;
	std::string		getHost() const;
	std::string		getConnection() const;
	std::string		getRequestedURL() const;
	std::string		getContentType() const;
	char* const*	getRequestEnv() const;
	int				getCGI() const;

	//------------------- OPERATORS ----------------------//

	Request &operator=(const Request &other);

	//--------------- MEMBER FUNCTIONS -------------------//

	void			decodeHTTPRequest(std::string &httpRequest);
	std::string		getInfoFromHTTPHeader(std::string &htmlRequest, std::string &infoType);
	void 			setCGI();
	void			redirectIfCGI();
	void			testFilename();
	void			handleCGI();

		//------------------ EXCEPTIONS ----------------------//

	class HTTPError : public std::exception
	{
	private:
		std::string	_message;

	public:
		HTTPError(Request& req, int status);
		~HTTPError() throw() {};
		std::string	getErrorPage();
	};

	class timeout : public std::exception {
		public :
			const char* what() const throw();
	};

	class badSyntax : public std::exception {
		public :
			const char* what() const throw();
	};

	class missingLength : public std::exception {
		public :
			const char* what() const throw();
	};

	class contentTooLarge : public std::exception {
		public :
			const char* what() const throw();
	};

	class forbiddenMethod : public std::exception {
		public :
			const char* what() const throw();
	};

	class badProtocol : public std::exception {
		public :
			const char* what() const throw();
	};

};
