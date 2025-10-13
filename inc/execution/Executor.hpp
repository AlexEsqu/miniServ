#pragma once

#include <iostream>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <cstdio> // for perror

#include "Response.hpp"
#include "Request.hpp"
#include "parsing.hpp"

enum e_pipe_fd
{
	READ = 0,
	WRITE = 1,
};

// Base class for all executor classes depending on language
// for example phpExecutor, pythonExecutor, etc.
class Executor
{

private:

public:

	// //----------------- CONSTRUCTORS ---------------------//

	// Executor();
	// Executor(const Executor& original);

	// //------------------- OPERATORS ----------------------//

	// Executor&	operator=(const Executor&);

	//----------------- DESTRUCTOR -----------------------//

	virtual ~Executor();

	//-------------- MEMBER FUNCTIONS --------------------//

	// create a fork to execute file into a pipe

	void			executeFile(Request& request);

	// create custom env using CGI to be used when executing the file

	std::string		formatKeyValueIntoSingleString(const std::string& key, const std::string& value);
	std::string		formatAsHTTPVariable(const std::string& headerKey, const std::string& headerValue);
	void			addCGIEnvironment(std::vector<std::string> envAsStrVec, const Request& request);
	std::vector<std::string>	generateEnvStrVec(Request& request);

	// read result from other end of pipe, put it into response content

	void			addResultToContent(Response &response, int fd);

	//-------------- ABSTRACT FUNCTIONS --------------------//

	virtual void	execFileWithFork(Request& request, const std::string& filePath, int* pipefd) = 0;
	virtual bool	canExecuteFile(const std::string& filePath) const = 0;

};
