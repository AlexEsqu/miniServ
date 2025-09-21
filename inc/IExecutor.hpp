#pragma once

class Response;

// Interface for all executor function depending on binary
// for example phpexecutor, pythonExecutor, etc.
class Executor
{

private:

public:

	//----------------- CONSTRUCTORS ---------------------//

	//----------------- DESTRUCTOR -----------------------//

	virtual ~Executor() {};

	//------------------- OPERATORS ----------------------//

	virtual void	executeFile(Response& response) = 0;
	virtual bool	canExecuteFile(Response& response) = 0;

};
