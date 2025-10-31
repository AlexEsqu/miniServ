#pragma once

#include "Executor.hpp"

class PythonExecutor : public Executor
{
private:

	std::vector<std::string>	_envAsStr;
	std::string					_filePath;

	std::vector<const char*>	buildArgv(const char* program, const std::string& filePath);
	std::vector<const char*>	buildEnv(Request& request);
	void						execFileWithFork(ClientSocket* client, int* pipefd);

public:

	//----------------- CONSTRUCTORS ---------------------//

	PythonExecutor();
	PythonExecutor(const PythonExecutor&);

	//----------------- DESTRUCTOR -----------------------//

	~PythonExecutor();

	//------------------- OPERATORS ----------------------//

	PythonExecutor&	operator=(const PythonExecutor&);

	//--------------- MEMBER FUNCTION --------------------//

	bool	canExecuteFile(const std::string& filePath) const;

};
