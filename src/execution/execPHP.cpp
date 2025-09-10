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


int	execFileWithPHP(std::string& fileToExecPath)
{
	const char*		program = "/usr/bin/php";
	const char*		flag = "-f";

	// redirect into pipe

	// expand (if needed ?)

	// unchunk (if needed ?)

	// add EOF at end of file path

	std::vector<const char*> argv(buildArgv(program, flag, fileToExecPath));

	execve("/usr/bin/php", (char* const*)argv.data(), environ);

	// clean up

	return (-1);
}

int	execPHPwithFork(std::string& fileToExecPath)
{
	int	fork_pid;
	int	exit_code;

	exit_code = 0;
	fork_pid = fork();
	if (fork_pid == -1)
		return (-1);
	if (fork_pid == 0)
		execFileWithPHP(fileToExecPath);
	else
		waitpid(fork_pid, &exit_code, 0);
	exit_code = WEXITSTATUS(exit_code);
	return (exit_code);
}
