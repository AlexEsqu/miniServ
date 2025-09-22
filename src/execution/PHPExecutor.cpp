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

std::vector<const char*>	PHPExecutor::buildEnv(Response& response)
{
	std::vector<std::string>	envAsStr = generateEnvStrVec(response);

	std::vector<const char*>	env;
	env.reserve(envAsStr.size() + 1);
	for (size_t i = 0; i != envAsStr.size(); i++)
	{
		env.push_back(envAsStr[i].c_str());
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


void	PHPExecutor::execFileWithFork(Response& response, const std::string& fileToExecPath, int* pipefd)
{
	const char*		program = "/usr/bin/php";
	const char*		flag = "-f";

	// redirect into pipe
	close(pipefd[READ]);
	if (dup2(pipefd[WRITE], STDOUT_FILENO) == -1)
		exit(-1);
	close(pipefd[WRITE]);

	// expand (if needed ?)

	// unchunk (if needed ?)

	// assemble into an execve approved array of char*, add EOF at end
	std::vector<const char*> argv(buildArgv(program, flag, fileToExecPath));
	std::vector<const char*> env(buildEnv(response));

	execve(program, (char**)argv.data(), (char**)env.data());

	// error handling
	perror("execve");

	// clean up

	exit(-1);
}

bool	PHPExecutor::canExecuteFile(Response& response)
{
	const char*	allowedExtension = ".php";

	std::size_t pos = response.getRoutedURL().find(allowedExtension);
	if (pos == std::string::npos)
		return false;
	else
		return true;
}
