#pragma once

#include <iostream>
#include <sys/stat.h>

#include "Response.hpp"
#include "Executor.hpp"
#include "ClientSocket.hpp"
#include "Poller.hpp"

class ContentFetcher
{

private:

	//------------------ ATTRIBUTES ----------------------//

	std::vector<Executor*>	_executors;
	Poller*					_poller;		// epoll instance object used to watch pipe if executing CGI

	//--------------- MEMBER FUNCTIONS -------------------//

	bool			isDirectory(const char *path);
	bool			isExisting(const char *path);
	bool			isAllowed(const char *path);
	size_t			getSizeOfFile(const std::string& filename);
	std::string		getTypeBasedOnExtension(const std::string& filePath);

public:

	//----------------- CONSTRUCTORS ---------------------//

	ContentFetcher(Poller* poller);
	ContentFetcher(const ContentFetcher &copy);

	//----------------- DESTRUCTOR -----------------------//

	~ContentFetcher();

	//------------------- OPERATORS ----------------------//

	ContentFetcher&	operator=(const ContentFetcher &other);

	//--------------- MEMBER FUNCTION --------------------//

	void			fillResponse(Request* request);

	// GET method

	void			getItemFromServer(Request& request);
	void			serveStatic(Request& request);

	// POST method

	void			postItemFromServer(Request& request);
	void			handleFormSubmission(Request& request);
	void			handleFileUpload(Request& request);

	// DELETE method

	void			deleteItemFromServer(Request& request);

	// Execution

	void			addExecutor(Executor* executor);
	e_dataProgress	readCGIChunk(Request& request, int pipeFd);

};
