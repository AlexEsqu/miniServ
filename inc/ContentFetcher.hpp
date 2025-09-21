#pragma once

#include <iostream>
#include <sys/stat.h>

#include "Response.hpp"
#include "EnvironmentBuilder.hpp"
#include "IExecutor.hpp"

class ContentFetcher
{

private:

	//------------------ ATTRIBUTES ----------------------//

	std::vector<Executor*>	executors;

	//--------------- MEMBER FUNCTIONS -------------------//

	bool			isDirectory(const char *path);
	bool			isExisting(const char *path);
	bool			isAllowed(const char *path);

	void			executeIfCGI(Response& response);
	void			serveStatic(Response& response);

public:

	//----------------- CONSTRUCTORS ---------------------//

	ContentFetcher();
	ContentFetcher(const ContentFetcher &copy);

	//----------------- DESTRUCTOR -----------------------//

	~ContentFetcher();

	//------------------- OPERATORS ----------------------//

	ContentFetcher&	operator=(const ContentFetcher &other);

	//--------------- MEMBER FUNCTION --------------------//

	void			addExecutor(Executor* executor);
	void			fillContent(Response& response);

};
