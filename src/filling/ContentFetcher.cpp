#include "ContentFetcher.hpp"

//----------------- CONSTRUCTORS ---------------------//

ContentFetcher::ContentFetcher()
{
}

ContentFetcher::ContentFetcher(const ContentFetcher &original)
{
	if (this != &original)
	{
		_executors = original._executors;
	}
	*this = original;
}

ContentFetcher &ContentFetcher::operator=(const ContentFetcher &original)
{
	if (this != &original)
	{
		_executors = original._executors;
	}
	return *this;
}

//----------------- DESTRUCTOR -----------------------//

ContentFetcher::~ContentFetcher()
{
	for (std::vector<Executor *>::iterator item = _executors.begin(); item != _executors.end(); item++)
		delete *item;
}

//----------------- INTERNAL ---------------------//

void		ContentFetcher::addExecutor(Executor *executor)
{
	_executors.push_back(executor);
}

//------------------- MEMBER FUNCTIONS ------------------------//

std::string	ContentFetcher::getTypeBasedOnExtension(const std::string &filePath)
{
	size_t dotPos = filePath.find_last_of('.');
	if (dotPos == std::string::npos)
		return "text/plain";

	std::string extension = filePath.substr(dotPos + 1);

	std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
	if (extension == "html" || extension == "htm")
		return "text/html";
	if (extension == "css")
		return "text/css";
	if (extension == "js")
		return "application/javascript";
	if (extension == "jpg" || extension == "jpeg")
		return "image/jpeg";
	if (extension == "png")
		return "image/png";
	if (extension == "gif")
		return "image/gif";
	if (extension == "svg")
		return "image/svg+xml";
	if (extension == "ico")
		return "image/x-icon";
	if (extension == "txt")
		return "text/plain";
	if (extension == "pdf")
		return "application/pdf";
	if (extension == "webp")
		return "image/webp";
	if (extension == "png")
		return "image/png";

	return "text/plain";
}

size_t		ContentFetcher::getSizeOfFile(const std::string &filename)
{
	struct stat st;
	if (stat(filename.c_str(), &st) != 0)
	{
		return 0;
	}
	return st.st_size;
}

bool		ContentFetcher::isDirectory(const char *path)
{
	struct stat path_stat;
	if (stat(path, &path_stat) != 0)
		return false;
	return S_ISDIR(path_stat.st_mode);
}

void		ContentFetcher::serveErrorPage(ClientSocket *client, e_status status)
{
	client->getRequest()->setError(status);
	client->getResponse()->createHTTPHeaders();
	client->setClientState(CLIENT_HAS_FILLED);
}

void		ContentFetcher::serveErrorPageBasedOnExistingStatus(ClientSocket *client)
{
	client->getResponse()->createHTTPHeaders();
	client->setClientState(CLIENT_HAS_FILLED);
}

void		ContentFetcher::fillResponse(ClientSocket *client)
{
	// create a response object and attach it to the request
	client->createNewResponse();
	Request *request = client->getRequest();
	Router::routeRequest(client->getRequest(), client->getResponse());

	verboseLog("Filling request to: " + client->getResponse()->getRoutedURL());

	// if an error has been caught when parsing, no need to fetch content
	if (request->hasError())
		serveErrorPageBasedOnExistingStatus(client);

	// else use the correct function to execute the requested method
	else if (request->getMethodCode() == GET)
		getItemFromServer(client);
	else if (request->getMethodCode() == POST)
		postItemFromServer(client);
	else if (request->getMethodCode() == DELETE)
		deleteItemFromServer(client);
	// else if (request.getMethodCode() == PUT)
	//	postItemFromServer(client);
	// else if (request.getMethodCode() == HEAD)
	//	getItemFromServer(client);
}


