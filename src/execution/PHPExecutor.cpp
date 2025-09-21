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

std::vector<const char*> PHPExecutor::buildArgv(const char* program, const char* flag, const std::string& filePath)
{
	std::vector<const char*> argv;

	argv.push_back(program);
	argv.push_back(flag);
	argv.push_back(filePath.c_str());
	// for (const std::string& arg : args) {
	// 	argv.push_back(arg.c_str());
	// }
	argv.push_back(NULL);
	return argv;
}


void	PHPExecutor::execFileWithFork(Response& Response, const std::string& fileToExecPath, int* pipefd)
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

	EnvironmentBuilder	EnvironmentBuilder(Response.getRequest());
	Environment			env = EnvironmentBuilder.generatePHPEnv();

	execve(program, (char* const*)argv.data(), env.getEnv());

	perror("execve");
	// clean up

	exit(-1);
}


void	PHPExecutor::executeFile(Response& response)
{
	int	fork_pid;
	int	pipefd[2];
	int	exit_code = 0;
	std::string	s = "";

	if (access(response.getRoutedURL().c_str(), O_RDONLY) != 0)
		return;

	if (pipe(pipefd) != 0)
		return;
	fork_pid = fork();
	if (fork_pid == -1)
		return;

	if (fork_pid == 0)
		execFileWithFork(response, response.getRoutedURL(), pipefd);

	else {
		close(pipefd[WRITE]);

		char	buff;
		while (read(pipefd[READ], &buff, 1) > 0)
		{
			if (buff != 0)
				s.push_back(buff);
		}

		std::cout << "Pipe read was : [" << s << "]\n";
		response.setContent(s);
		close(pipefd[READ]);
	}
	std::cerr << ERROR_FORMAT("out of reading loop\n");
	waitpid(fork_pid, &exit_code, 0);
	std::cerr << ERROR_FORMAT("fork waited\n");
	exit_code = WEXITSTATUS(exit_code);
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
