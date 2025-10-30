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

std::vector<const char*>	PythonExecutor::buildEnv(Request& request)
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

std::vector<const char*> PythonExecutor::buildArgv(const char* program, const std::string& filePath)
{
	std::vector<const char*> argv;

	argv.push_back(program);
	// argv.push_back(flag);
	argv.push_back(filePath.c_str());
	argv.push_back(NULL);
	return argv;
}


void	PythonExecutor::execFileWithFork(ClientSocket* client, int* pipefd)
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
	std::vector<const char*> argv(buildArgv(program, client->getResponse().getRoutedURL()));
	std::vector<const char*> env(buildEnv(client->getRequest()));

	execve(program, (char**)argv.data(), (char**)env.data());

	// error handling
	perror("execve");

	// clean up

	exit(-1);
}

bool	PythonExecutor::canExecuteFile(const std::string& filePath) const
{
	const char*	allowedExtension = ".py";

	std::size_t pos = filePath.find(allowedExtension);
	if (pos == std::string::npos)
		return false;
	else
		return true;
}
