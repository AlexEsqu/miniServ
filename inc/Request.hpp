#pragma once
#include <iostream>
#include <string>
#include <algorithm>
#include <map>
#include <sstream>

#include "readability.hpp"
#include "EnvironmentBuilder.hpp"

class Request;

class Request
{

private:

	//------------------ ATTRIBUTES ----------------------//

	std::string			_fullRequest;		// full content of the request
	std::string			_method;			// could be set as the enum already ?
	std::string			_protocol;			// probably not needed, unless we only support HTTP/1.1
	std::string			_host;				// for example "example.com"
	std::string			_connection;		// to research keep-alive
	std::string			_requestedFileName;	// for example "/home.html"
	std::string			_contentType;
	int					_CGI;				// 0 NO CGI, 1 PY, 2 PHP
	EnvironmentBuilder	_requestEnv;

	//-------------- INTERNAL FUNCTIONS -------------------//

	void			checkHTTPValidity();

public:

	//----------------- CONSTRUCTORS ---------------------//

	Request(); // empty constructor for testing purposes
	Request(std::string httpRequest);
	Request(const Request &copy);

	//----------------- DESTRUCTOR -----------------------//

	~Request();

	//-------------------- SETTER ------------------------//

	void			setMethod(std::string& httpRequest);
	void			setProtocol(std::string& httpRequest);
	void			setURI(std::string& httpRequest);
	void			setRequestLine(std::string& httpRequest);
	void			setRequestEnv(std::string& keyValueString);

	//-------------------- GETTERS -----------------------//

	std::string		getMethod() const;
	std::string		getProtocol() const;
	std::string		getHost() const;
	std::string		getConnection() const;
	std::string		getRequestedURL() const;
	std::string		getContentType() const;
	Environment		getRequestEnv();
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

	class timeout : public HTTPError {
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


