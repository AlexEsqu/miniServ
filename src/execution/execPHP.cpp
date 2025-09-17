# include "server.hpp"

std::vector<const char*> buildArgv(const char* program, const char* flag, std::string& filePath)
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


int	execFileWithPHP(Request& request, std::string& fileToExecPath, int* pipefd)
{
	const char*		program = "/usr/bin/php";
	const char*		flag = "-f";

	// redirect into pipe
	close(pipefd[READ]);
	if (dup2(pipefd[WRITE], STDOUT_FILENO) == -1)
		return (-1);
	close(pipefd[WRITE]);

	// expand (if needed ?)

	// unchunk (if needed ?)

	// assemble into an execve approved array of char*, add EOF at end
	std::vector<const char*> argv(buildArgv(program, flag, fileToExecPath));

	Environment	env = request.getRequestEnv();

	execve(program, (char* const*)argv.data(), env.getEnv());

	// clean up

	return (-1);
}

std::string	execPHPwithFork(Request& request, std::string& fileToExecPath)
{
	int	fork_pid;
	int	pipefd[2];
	int	exit_code = 0;
	std::string	s = "";

	if (access(fileToExecPath.c_str(), O_RDONLY) != 0)
		return (s);

	if (pipe(pipefd) != 0)
		return (s);
	fork_pid = fork();
	if (fork_pid == -1)
		return (s);

	if (fork_pid == 0)
		execFileWithPHP(request, fileToExecPath, pipefd);

	else {
		close(pipefd[WRITE]);

		char	buff;
		while (read(pipefd[READ], &buff, 1) > 0)
		{
			if (buff != 0)
				s.push_back(buff);
		}

		std::cout << "Pipe read was : [" << s << "]\n";

		close(pipefd[READ]);
	}
	std::cerr << RED << "out of reading loop\n" << STOP_COLOR;
	waitpid(fork_pid, &exit_code, 0);
	std::cerr << RED << "fork waited\n" << STOP_COLOR;
	exit_code = WEXITSTATUS(exit_code);
	return (s);
}
