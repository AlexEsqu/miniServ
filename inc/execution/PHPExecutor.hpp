#pragma once
#include <iostream>

#include "Executor.hpp"

class PHPExecutor : public Executor
{

private:

	std::vector<const char*>	buildArgv(const char* program, const char* flag, const std::string& filePath);
	std::vector<const char*>	buildEnv(Response& response);
	void	execFileWithFork(Response& response, const std::string& filePath, int* pipefd);

public:

	//----------------- CONSTRUCTORS ---------------------//

	PHPExecutor();
	PHPExecutor(const PHPExecutor&);

	//----------------- DESTRUCTOR -----------------------//

	~PHPExecutor();

	//------------------- OPERATORS ----------------------//

	PHPExecutor&	operator=(const PHPExecutor&);

	//--------------- MEMBER FUNCTION --------------------//

	void	executeFile(Response& response);
	bool	canExecuteFile(Response& response);

};
