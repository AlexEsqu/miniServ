#pragma once
#include <iostream>

#include "Response.hpp"
#include "IExecutor.hpp"
#include "parsing.hpp"

class PythonExecutor : public Executor
{

private:

	std::string	formatKeyValueIntoSingleString(const std::string& key, const std::string& value);
	std::string	formatAsHTTPVariable(const std::string& headerKey, const std::string& headerValue);
	void		addCGIEnvironment(std::vector<std::string> envAsStrVec, const Request& request);
	std::vector<std::string>	generatePHPEnvStrVec(Response& response);
	void		readResultIntoContent(Response& response, int fd);

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
