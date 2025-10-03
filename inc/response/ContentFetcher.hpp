#pragma once

#include <iostream>
#include <sys/stat.h>

#include "Response.hpp"
#include "Executor.hpp"
#include "ClientSocket.hpp"

class ContentFetcher
{

private:

	//------------------ ATTRIBUTES ----------------------//

	std::vector<Executor*>	executors;

	//--------------- MEMBER FUNCTIONS -------------------//

	bool			isDirectory(const char *path);
	bool			isExisting(const char *path);
	bool			isAllowed(const char *path);
	size_t			getSizeOfFile(const std::string& filename);
	std::string		getTypeBasedOnExtension(const std::string& filePath);

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
	void			fetchPage(Response& response);

};
