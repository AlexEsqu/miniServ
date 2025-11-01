#include "PHPExecutor.hpp"

PHPExecutor::PHPExecutor()
{

}

PHPExecutor::PHPExecutor(const PHPExecutor&)
{

}


//----------------- DESTRUCTOR -----------------------//

PHPExecutor::~PHPExecutor()
{

}


//------------------- OPERATORS ----------------------//

PHPExecutor& PHPExecutor::operator=(const PHPExecutor&)
{
	return (*this);
}

//---------------- MEMBER FUNCTION -------------------//

std::vector<const char*>	PHPExecutor::buildEnv(Request& request)
{
	_envAsStr.clear();

	_envAsStr = generateEnvStrVec(request);

	std::vector<const char*>	env;
	env.reserve(_envAsStr.size() + 1);
	for (size_t i = 0; i != _envAsStr.size(); i++)
	{
		env.push_back(_envAsStr[i].c_str());
	}
	env.push_back(NULL);

	return env;
}

std::vector<const char*> PHPExecutor::buildArgv(const char* program, const char* flag, const std::string& filePath)
{
	std::vector<const char*> argv;

	argv.push_back(program);
	argv.push_back(flag);
	argv.push_back(filePath.c_str());
	argv.push_back(NULL);
	return argv;
}


void	PHPExecutor::execFileWithFork(ClientSocket* client, int* pipefd)
{
	const char*		program = "/usr/bin/php-cgi";
	const char*		flag = "-f";
	std::string		executedFile = client->getResponse().getRoutedURL();

	// redirect into pipe
	close(pipefd[READ]);
	if (dup2(pipefd[WRITE], STDOUT_FILENO) == -1)
		exit(-1);
	close(pipefd[WRITE]);

	// // redirecting to the body buffer if post request
	// if (client->getRequest().getMethodCode() == POST)
	// {
	// 	int bodyFd = client->getRequest().getStreamFromBodyBuffer();
	// 	if (dup2(bodyFd, STDIN_FILENO) == -1)
	// 		exit(-1);
	// 	close(bodyFd);
	// }

	// assemble into an execve approved array of char*, add EOF at end
	std::vector<const char*> argv(buildArgv(program, flag, executedFile));
	std::vector<const char*> env(buildEnv(client->getRequest()));

	execve(program, (char**)argv.data(), (char**)env.data());

	throw std::runtime_error("exec failed");
}

bool	PHPExecutor::canExecuteFile(const std::string& filePath) const
{
	const char*	allowedExtension = ".php";

	std::size_t pos = filePath.find(allowedExtension);
	if (pos == std::string::npos)
		return false;
	else
		return true;
}
