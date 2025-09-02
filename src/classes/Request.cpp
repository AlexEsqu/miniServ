#include "Request.hpp"

//--------------------------- CONSTRUCTORS ----------------------------------//

Request::Request(std::string httpRequest)
	: _fullRequest(httpRequest)
	, _method(extractMethodFromHTTP(_fullRequest))
{
	#ifdef DEBUG
		std::cout << "Request Constructor called" << std::endl;
		std::cout << "Method is " << _method << std::endl;
	#endif



}

Request::Request(const Request &copy)
{
	#ifdef DEBUG
		std::cout << "Request copy Constructor called" << std::endl;
	#endif
	*this = copy;
}

//--------------------------- DESTRUCTORS -----------------------------------//

Request::~Request()
{
	#ifdef DEBUG
		std::cout << "Request Destructor called" << std::endl;
	#endif
}

//---------------------------- OPERATORS ------------------------------------//

Request &Request::operator=(const Request &other)
{
	if (this == &other)
		return *this;
	return *this;
}

//---------------------------- GUETTERS -------------------------------------//


//---------------------------- SETTERS --------------------------------------//


//------------------------ MEMBER FUNCTIONS ---------------------------------//

std::string	Request::extractMethodFromHTTP(std::string httpRequest)
{
	std::string	httpMethod = "";

	for (std::string::iterator it = httpRequest.begin(); *it != ' '; it++) {
		httpMethod.append(1, *it);
	}
	return (httpMethod);
}
