#pragma once

#include <iostream>
#include <sys/stat.h>
#include <iomanip>
#include <algorithm>
#include <dirent.h>

#include "Response.hpp"
#include "Executor.hpp"
#include "ServerSocket.hpp"
#include "ClientSocket.hpp"
#include "Router.hpp"

class Poller;
class Executor;
enum	e_mutipartState
{
	MP_STARTING_LINE,
	MP_HEADERS,
	MP_CONTENT,
	MP_FINAL_BOUNDARY
};


class ContentFetcher
{

private:

	//------------------ ATTRIBUTES ----------------------//

	std::vector<Executor*>	_executors;

	//--------------- MEMBER FUNCTIONS -------------------//

	size_t			getSizeOfFile(const std::string& filename);
	std::string		getTypeBasedOnExtension(const std::string& filePath);
	std::string 	getExtensionFromType(const std::string& type);
	std::string		findFileInDirectory(std::string directory, std::string filename);

public:

	//----------------- CONSTRUCTORS ---------------------//

	ContentFetcher();
	ContentFetcher(const ContentFetcher &copy);

	//----------------- DESTRUCTOR -----------------------//

	~ContentFetcher();

	//------------------- OPERATORS ----------------------//

	ContentFetcher&	operator=(const ContentFetcher &other);

	//--------------- MEMBER FUNCTION --------------------//

	void			fillResponse(ClientSocket* client);
	void			serveErrorPage(ClientSocket* client, e_status status);
	void			serveErrorPageBasedOnExistingStatus(ClientSocket* client);
	void			openSessionDirectory(ClientSocket *client);

	// GET method

	void			getItemFromServer(ClientSocket* client);
	void			serveStatic(ClientSocket* client);
	void			serveDirectoryListing(ClientSocket* client, std::string& fileURL);
	std::string		createDirectoryListing(const std::string& path, const std::string& requestUri);

	// POST method

	void			postItemFromServer(ClientSocket* client);
	void			parseBodyDataAndUpload(ClientSocket* client);
	void			parseUrlEncodedBody(ClientSocket* client);
	void			parseMultiPartBody(ClientSocket* client);
	void			parsePlainBody(ClientSocket *client);
	void			createPostResponsePage(ClientSocket* client);

	static std::string		extractBoundary(const std::string& contentType);

	// DELETE method

	void			deleteItemFromServer(ClientSocket* client);

	// HEAD method

	void			headItemFromServer(ClientSocket* client);

	// Execution

	void			addExecutor(Executor* executor);
	e_dataProgress	readCGIChunk(ClientSocket* client);

};
