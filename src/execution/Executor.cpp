#include "Executor.hpp"

// //----------------- CONSTRUCTOR -----------------------//

// Executor::Executor()
// {

// }

// Executor::Executor(const Executor& original)
// {
// 	*this = original;
// }


//----------------- DESTRUCTOR -----------------------//

Executor::~Executor()
{

}


// //------------------- OPERATORS ----------------------//

// Executor& Executor::operator=(const Executor& original)
// {
// 	if (this != &original)
// 		return *this;
// 	return *this;
// }

//---------------- MEMBER FUNCTION -------------------//

void	Executor::readResultIntoContent(Response& response, int fd)
{
	std::string	s = "";

	char	buff;
	while (read(fd, &buff, 1) > 0)
	{
		if (buff != 0)
			s.push_back(buff);
	}
	#ifdef DEBUG
		std::cout << "Pipe read was : [" << s << "]\n";
	#endif

	response.setContent(s);
}

std::vector<std::string>	Executor::generateEnvStrVec(Response& response)
{
	std::vector<std::string>	envAsStrVec;

	addCGIEnvironment(envAsStrVec, response.getRequest());

	std::map<std::string, std::string>::const_iterator item;
	for (item = response.getRequest().getAdditionalHeaderInfo().begin();
		item != response.getRequest().getAdditionalHeaderInfo().end(); item++)
	{
		envAsStrVec.push_back(formatAsHTTPVariable(item->first, item->second));
	}

	return envAsStrVec;
}

std::string	Executor::formatAsHTTPVariable(const std::string& key, const std::string& value)
{
	std::string	formattedEnvKeyValue = "";

	// Replace hyphens with underscores and convert to uppercase
	std::string	formattedKey = key;
	for (size_t i = 0; i < key.length(); ++i) {
		if (key[i] == '-' || key[i] == ' ') {
			formattedKey[i] = '_';
		}
	}
	strToUpper(formattedKey);

	// Convert HTTP headers to CGI format: HTTP_HEADER_NAME
	if (formattedKey == "CONTENT_TYPE" || formattedKey == "CONTENT_LENGTH")
		formattedEnvKeyValue = formattedKey + "=" + value;
	else
		formattedEnvKeyValue = "HTTP_" + formattedKey + "=" + value;

	return (formattedEnvKeyValue);
}

std::string	Executor::formatKeyValueIntoSingleString(const std::string& key, const std::string& value)
{
	std::string	formattedEnvKeyValue = key + "=" + value;
	return formattedEnvKeyValue;
}

void Executor::addCGIEnvironment(std::vector<std::string> envAsStrVec, const Request& request)
{
	// Standard CGI variables
	envAsStrVec.push_back(formatKeyValueIntoSingleString("REQUEST_METHOD", request.getMethod()));
	envAsStrVec.push_back(formatKeyValueIntoSingleString("REQUEST_URI", request.getRequestedURL()));
	envAsStrVec.push_back(formatKeyValueIntoSingleString("SERVER_PROTOCOL", request.getProtocol()));

	// Server information
	envAsStrVec.push_back(formatKeyValueIntoSingleString("SERVER_NAME", "localhost"));		// or from config
	envAsStrVec.push_back(formatKeyValueIntoSingleString("SERVER_PORT", "8080"));			// or from config
}
