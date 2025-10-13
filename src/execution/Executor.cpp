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

void	Executor::addResultToContent(Response &response, int fd)
{
	std::string	s = "";

	char	buff;
	while (read(fd, &buff, 1) > 0)
	{
		if (buff != 0)
			s.push_back(buff);
	}
	// #ifdef DEBUG
	// 	std::cout << "Pipe read was : [" << s << "]\n";
	// #endif

	response.addToContent(s);
}

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
	std::string	formattedEnvKeyValue = "";

	// Replace hyphens with underscores and convert to uppercase
	std::string	formattedKey = key;
	for (size_t i = 0; i < key.length(); ++i) {
		if (key[i] == '-' || key[i] == ' ') {
			formattedKey[i] = '_';
		}
	}
	strToUpper(formattedKey);

	std::string	formattedValue = value;
	for (size_t i = 0; i < value.length(); ++i) {
		if (value[i] == '-' || value[i] == ' ') {
			formattedValue[i] = '_';
		}
		// TO DO : add encoding for non variable compliant characters such as ", ', % ....
	}
	strToUpper(formattedValue);

	// Convert HTTP headers to CGI format: HTTP_HEADER_NAME
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

void Executor::addCGIEnvironment(std::vector<std::string> envAsStrVec, const Request& request)
{
	// Standard CGI variables
	envAsStrVec.push_back(formatKeyValueIntoSingleString("REQUEST_METHOD", request.getMethod()));
	envAsStrVec.push_back(formatKeyValueIntoSingleString("REQUEST_URI", request.getRequestedURL()));
	envAsStrVec.push_back(formatKeyValueIntoSingleString("SERVER_PROTOCOL", request.getProtocol()));
	// TO DO : add query string (but CGI is unchunking on his own)

	// Server information
	envAsStrVec.push_back(formatKeyValueIntoSingleString("SERVER_NAME", "localhost"));		// or from config
	envAsStrVec.push_back(formatKeyValueIntoSingleString("SERVER_PORT", "8080"));			// or from config
}

void	Executor::executeFile(Request& request)
{
	int	fork_pid;
	int	pipefd[2];
	int	exit_code = 0;

	if (pipe(pipefd) != 0)
		return;
	fork_pid = fork();
	if (fork_pid == -1)
		return;

	if (fork_pid == 0)
		execFileWithFork(request, request.getResponse()->getRoutedURL(), pipefd);

	else {
		close(pipefd[WRITE]);
		addResultToContent(*(request.getResponse()), pipefd[READ]);
		close(pipefd[READ]);
	}

	waitpid(fork_pid, &exit_code, 0);
	exit_code = WEXITSTATUS(exit_code);
}
