#include "PythonExecutor.hpp"

PythonExecutor::PythonExecutor()
{

}

PythonExecutor::PythonExecutor(const PythonExecutor&)
{

}


//----------------- DESTRUCTOR -----------------------//

PythonExecutor::~PythonExecutor()
{

}


//------------------- OPERATORS ----------------------//

PythonExecutor& PythonExecutor::operator=(const PythonExecutor&)
{
	return (*this);
}

//---------------- MEMBER FUNCTION -------------------//

void	PythonExecutor::readResultIntoContent(Response& response, int fd)
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

std::vector<std::string>	PythonExecutor::generatePHPEnvStrVec(Response& response)
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

std::string	PythonExecutor::formatAsHTTPVariable(const std::string& key, const std::string& value)
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

std::string	PythonExecutor::formatKeyValueIntoSingleString(const std::string& key, const std::string& value)
{
	std::string	formattedEnvKeyValue = key + "=" + value;
	return formattedEnvKeyValue;
}

void PythonExecutor::addCGIEnvironment(std::vector<std::string> envAsStrVec, const Request& request)
{
	// Standard CGI variables
	envAsStrVec.push_back(formatKeyValueIntoSingleString("REQUEST_METHOD", request.getMethod()));
	envAsStrVec.push_back(formatKeyValueIntoSingleString("REQUEST_URI", request.getRequestedURL()));
	envAsStrVec.push_back(formatKeyValueIntoSingleString("SERVER_PROTOCOL", request.getProtocol()));

	// Server information
	envAsStrVec.push_back(formatKeyValueIntoSingleString("SERVER_NAME", "localhost"));		// or from config
	envAsStrVec.push_back(formatKeyValueIntoSingleString("SERVER_PORT", "8080"));			// or from config
}

std::vector<const char*>	PythonExecutor::buildEnv(Response& response)
{
	std::vector<std::string>	envAsStr = generatePHPEnvStrVec(response);

	std::vector<const char*>	env;
	env.reserve(envAsStr.size() + 1);
	for (size_t i = 0; i != envAsStr.size(); i++)
	{
		env.push_back(envAsStr[i].c_str());
	}
	env.push_back(NULL);

	return env;
}

std::vector<const char*> PythonExecutor::buildArgv(const char* program, const std::string& filePath)
{
	std::vector<const char*> argv;

	argv.push_back(program);
	// argv.push_back(flag);
	argv.push_back(filePath.c_str());
	argv.push_back(NULL);
	return argv;
}


void	PythonExecutor::execFileWithFork(Response& response, const std::string& fileToExecPath, int* pipefd)
{
	const char*		program = "/usr/bin/python3";

	// redirect into pipe
	close(pipefd[READ]);
	if (dup2(pipefd[WRITE], STDOUT_FILENO) == -1)
		exit(-1);
	close(pipefd[WRITE]);

	// expand (if needed ?)

	// unchunk (if needed ?)

	// assemble into an execve approved array of char*, add EOF at end
	std::vector<const char*> argv(buildArgv(program, fileToExecPath));
	std::vector<const char*> env(buildEnv(response));

	execve(program, (char**)argv.data(), (char**)env.data());

	// error handling
	perror("execve");

	// clean up

	exit(-1);
}

void	PythonExecutor::executeFile(Response& response)
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
		execFileWithFork(response, response.getRoutedURL(), pipefd);

	else {
		close(pipefd[WRITE]);
		readResultIntoContent(response, pipefd[READ]);
		close(pipefd[READ]);
	}

	waitpid(fork_pid, &exit_code, 0);
	exit_code = WEXITSTATUS(exit_code);
}

bool	PythonExecutor::canExecuteFile(Response& response)
{
	const char*	allowedExtension = ".py";

	std::size_t pos = response.getRoutedURL().find(allowedExtension);
	if (pos == std::string::npos)
		return false;
	else
		return true;
}
