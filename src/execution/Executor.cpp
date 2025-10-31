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

std::vector<std::string>	Executor::generateEnvStrVec(Request& request)
{
	std::vector<std::string>	envAsStrVec;

	addCGIEnvironment(envAsStrVec, request);

	std::map<std::string, std::string>::const_iterator item;
	for (item = request.getAdditionalHeaderInfo().begin();
		item != request.getAdditionalHeaderInfo().end(); item++)
	{
		envAsStrVec.push_back(formatAsHTTPVariable(item->first, item->second));
	}

	return envAsStrVec;
}

std::string	Executor::formatAsHTTPVariable(const std::string& key, const std::string& value)
{
	std::string	formattedEnvKeyValue;

	// replacing hyphens with underscores and converting to uppercase
	std::string	formattedKey = key;
	for (size_t i = 0; i < key.length(); ++i)
	{
		if (key[i] == '-' || key[i] == ' ')
			formattedKey[i] = '_';
	}
	strToUpper(formattedKey);

	std::string	formattedValue = value;
	for (size_t i = 0; i < value.length(); ++i)
	{
		if (value[i] == '-' || value[i] == ' ')
			formattedValue[i] = '_';
		// TO DO : add encoding for non compliant characters like ", ', % ....
	}
	strToUpper(formattedValue);

	// convertint HTTP headers to CGI format: HTTP_HEADER_NAME except for Content Type and Length
	if (formattedKey == "CONTENT_TYPE" || formattedKey == "CONTENT_LENGTH")
		formattedEnvKeyValue = formattedKey + "=" + formattedValue;
	else
		formattedEnvKeyValue = "HTTP_" + formattedKey + "=" + formattedValue;

	return (formattedEnvKeyValue);
}

std::string	Executor::formatKeyValueIntoSingleString(const std::string& key, const std::string& value)
{
	std::string	formattedEnvKeyValue = key + "=" + value;
	return formattedEnvKeyValue;
}

void	Executor::parseQueryParameters(std::map<std::string, std::string>& queryParamMap, const std::string& queryString)
{
	std::istringstream					stream(queryString);

	std::string keyValuePairString;
	while (std::getline(stream, keyValuePairString, '&'))
	{
		size_t equalPos = keyValuePairString.find('=');
		if (equalPos != std::string::npos)
		{
			std::string key = parseUrlEncoding(keyValuePairString.substr(0, equalPos));
			std::string value = parseUrlEncoding(keyValuePairString.substr(equalPos + 1));
			queryParamMap[key] = value;
		}
		else
		{
			std::string key = parseUrlEncoding(keyValuePairString);
			queryParamMap[key] = "";
		}
	}
}

void	Executor::addQueryParamAsEnvironment(std::vector<std::string>& envAsStrVec, const std::string& queryString)
{
	std::map<std::string, std::string> queryParamMap;

	parseQueryParameters(queryParamMap, queryString);
	for (std::map<std::string, std::string>::iterator i = queryParamMap.begin(); i != queryParamMap.end(); i++)
	{
		envAsStrVec.push_back(formatKeyValueIntoSingleString(i->first, i->second));
	}
}

void	Executor::addCGIEnvironment(std::vector<std::string>& envAsStrVec, const Request& request)
{
	// standard CGI variables
	envAsStrVec.push_back(formatKeyValueIntoSingleString("REQUEST_METHOD", request.getMethodAsString()));
	envAsStrVec.push_back(formatKeyValueIntoSingleString("REQUEST_URI", request.getRequestedURL()));
	envAsStrVec.push_back(formatKeyValueIntoSingleString("SERVER_PROTOCOL", request.getProtocol()));

	// server information
	envAsStrVec.push_back(formatKeyValueIntoSingleString("SERVER_NAME", request.getConf().getServerName()));
	std::stringstream	portAsStream;
	portAsStream << request.getConf().getPort();
	envAsStrVec.push_back(formatKeyValueIntoSingleString("SERVER_PORT", portAsStream.str()));

	envAsStrVec.push_back(formatKeyValueIntoSingleString("SCRIPT_NAME", request.getRequestedURL()));
	envAsStrVec.push_back(formatKeyValueIntoSingleString("SCRIPT_FILENAME", Router::joinPaths(request.getConf().getRoot(), request.getRequestedURL())));
	envAsStrVec.push_back(formatKeyValueIntoSingleString("REDIRECT_STATUS", "200"));

	// query parameters (originally stored in URI)
	std::string	cgiParam = request.getCgiParam();
	if (!cgiParam.empty())
	{
		envAsStrVec.push_back(formatKeyValueIntoSingleString("QUERY_STRING", cgiParam));
		// addQueryParamAsEnvironment(envAsStrVec, cgiParam);
	}
}

void	Executor::executeFile(ClientSocket* client)
{
	int	pipefd[2];

	if (pipe(pipefd) != 0)
		throw std::runtime_error("pipe failed");

	int forkPid = fork();
	client->getRequest().setCgiForkPid(forkPid);
	if (forkPid < 0)
		throw std::runtime_error("fork failed");

	if (forkPid == 0)
	{
		execFileWithFork(client, pipefd);
		throw std::runtime_error("execve failed");
	}
	else
	{
		close(pipefd[1]);
		client->startReadingPipe(pipefd[0]);
	}
}
