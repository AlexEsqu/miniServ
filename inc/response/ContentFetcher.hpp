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



public:

	//----------------- CONSTRUCTORS ---------------------//

	ContentFetcher();
	ContentFetcher(const ContentFetcher &copy);

	//----------------- DESTRUCTOR -----------------------//

	~ContentFetcher();

	//------------------- OPERATORS ----------------------//

	ContentFetcher&	operator=(const ContentFetcher &other);

	//--------------- MEMBER FUNCTION --------------------//

	void			fetchPage(Request& request, Response& response);
	void			executeIfCGI(Response& response);
	void			serveStatic(Response& response);
	const Route*	findMatchingRoute(Request& request) const;

	void			addExecutor(Executor* executor);
	Response		createPage(Request* request);

};
