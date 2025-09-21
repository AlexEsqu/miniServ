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

std::vector<const char*>	PythonExecutor::buildEnv(Response& response)
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
