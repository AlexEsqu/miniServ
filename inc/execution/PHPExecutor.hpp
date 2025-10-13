#pragma once
#include <iostream>

#include "Executor.hpp"

class PHPExecutor : public Executor
{

private:

	std::vector<const char*>	buildArgv(const char* program, const char* flag, const std::string& filePath);
	std::vector<const char*>	buildEnv(Request& request);
	void						execFileWithFork(Request& request, const std::string& filePath, int* pipefd);

public:

	//----------------- CONSTRUCTORS ---------------------//

	PHPExecutor();
	PHPExecutor(const PHPExecutor&);

	//----------------- DESTRUCTOR -----------------------//

	~PHPExecutor();

	//------------------- OPERATORS ----------------------//

	PHPExecutor&	operator=(const PHPExecutor&);

	//--------------- MEMBER FUNCTION --------------------//

	bool	canExecuteFile(const std::string& filePath) const;

};
