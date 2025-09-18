#pragma once

#include <string>
#include <exception>

#include "Request.hpp"
#include "Response.hpp"

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
