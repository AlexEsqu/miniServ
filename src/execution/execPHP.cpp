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


int	execFileWithPHP(std::string& fileToExecPath, int* pipefd)
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

	execve(program, (char* const*)argv.data(), environ);

	// clean up

	return (-1);
}

std::string	execPHPwithFork(std::string& fileToExecPath)
{
	int	fork_pid;
	int	pipefd[2];
	int	exit_code = 0;
	std::string	s = "";

	if (pipe(pipefd) != 0)
		return (s);
	fork_pid = fork();
	if (fork_pid == -1)
		return (s);
	if (fork_pid == 0)
		execFileWithPHP(fileToExecPath, pipefd);
	else {
		waitpid(fork_pid, &exit_code, 0);
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
	exit_code = WEXITSTATUS(exit_code);
	return (s);
}
