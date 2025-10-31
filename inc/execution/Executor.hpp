#pragma once

#include <iostream>
#include <sys/wait.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>

#include "Response.hpp"
#include "Request.hpp"
#include "parsing.hpp"
#include "ClientSocket.hpp"

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

	void				executeFile(ClientSocket* client);

	// create custom env using CGI to be used when executing the file

	std::string			formatKeyValueIntoSingleString(const std::string& key, const std::string& value);
	std::string			formatAsHTTPVariable(const std::string& headerKey, const std::string& headerValue);
	void				parseQueryParameters(std::map<std::string, std::string>& queryParamMap, const std::string& queryString);
	void				addCGIEnvironment(std::vector<std::string>& envAsStrVec, const Request& request);
	void				addQueryParamAsEnvironment(std::vector<std::string>& envAsStrVec, const std::string& queryString);
	std::vector
		<std::string>	generateEnvStrVec(Request& request);

	// read result from other end of pipe, put it into response content

	void				addResultToContent(Response &response, int fd);
	void				parseCgiHeader(Response &response, std::string& headers);

	//-------------- ABSTRACT FUNCTIONS --------------------//

	virtual void		execFileWithFork(ClientSocket* client, int* pipefd) = 0;
	virtual bool		canExecuteFile(const std::string& filePath) const = 0;

};
