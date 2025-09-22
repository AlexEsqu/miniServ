#pragma once

#include "Executor.hpp"

class PythonExecutor : public Executor
{
private:

	std::vector<const char*>	buildArgv(const char* program, const std::string& filePath);
	std::vector<const char*>	buildEnv(Response& response);
	void	execFileWithFork(Response& response, const std::string& filePath, int* pipefd);

public:

	//----------------- CONSTRUCTORS ---------------------//

	PythonExecutor();
	PythonExecutor(const PythonExecutor&);

	//----------------- DESTRUCTOR -----------------------//

	~PythonExecutor();

	//------------------- OPERATORS ----------------------//

	PythonExecutor&	operator=(const PythonExecutor&);

	//--------------- MEMBER FUNCTION --------------------//

	void	executeFile(Response& response);
	bool	canExecuteFile(Response& response);

};
