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

void ContentFetcher::addExecutor(Executor *executor)
{
	_executors.push_back(executor);
}

//------------------- MEMBER FUNCTIONS ------------------------//

std::string ContentFetcher::getTypeBasedOnExtension(const std::string &filePath)
{
	size_t dotPos = filePath.find_last_of('.');
	if (dotPos == std::string::npos)
		return "text/plain";

	std::string extension = filePath.substr(dotPos + 1);
	std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
	if (extension == "html" || extension == "htm")
		return "text/html";
	else if (extension == "css")
		return "text/css";
	else if (extension == "js")
		return "application/javascript";
	else if (extension == "jpg" || extension == "jpeg")
		return "image/jpeg";
	else if (extension == "png")
		return "image/png";
	else if (extension == "gif")
		return "image/gif";
	else if (extension == "svg")
		return "image/svg+xml";
	else if (extension == "ico")
		return "image/x-icon";
	else if (extension == "txt")
		return "text/plain";
	else if (extension == "pdf")
		return "application/pdf";
	else if (extension == "webp")
		return "image/webp";
	else if (extension == "doc")
		return "application/msword";
	else if (extension == "xls")
		return "application/vnd.ms-excel";
	else if (extension == "ppt")
		return "application/vnd.ms-powerpoint";
	else if (extension == "odt")
		return "application/vnd.oasis.opendocument.text";
	else if (extension == "pptx")
		return "application/vnd.openxmlformats-officedocument.presentationml.presentation";
	else if (extension == "xlsx")
		return "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
	else if (extension == "docx")
		return "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
	else if (extension == "json")
		return "application/json";
	else if (extension == "xml")
		return "application/xml";
	else if (extension == "zip")
		return "application/zip";
	else if (extension == "csv")
		return "text/csv";
	else
		return "text/plain";
}

std::string ContentFetcher::getExtensionFromType(const std::string &type)
{
	std::string extension;
	if (type == "application/json")
		extension = ".json";
	else if (type == "application/ld+json")
		extension = ".json";
	else if (type == "application/msword")
		extension = ".doc";
	else if (type == "application/pdf")
		extension = ".pdf";
	else if (type == "application/sql")
		extension = ".sql";
	else if (type == "application/vnd.api+json")
		extension = ".json";
	else if (type == "application/vnd.microsoft.portable-executable")
		extension = ".efi";
	else if (type == "application/vnd.ms-excel")
		extension = ".xls";
	else if (type == "application/vnd.ms-powerpoint")
		extension = ".ppt";
	else if (type == "application/vnd.oasis.opendocument.text")
		extension = ".odt";
	else if (type == "application/vnd.openxmlformats-officedocument.presentationml.presentation")
		extension = ".pptx";
	else if (type == "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet")
		extension = ".xlsx";
	else if (type == "application/vnd.openxmlformats-officedocument.wordprocessingml.document")
		extension = ".docx";
	else if (type == "application/x-www-form-urlencoded")
		extension = ".form-urlencoded";
	else if (type == "application/xml")
		extension = ".xml";
	else if (type == "application/zip")
		extension = ".zip";
	else if (type == "application/zstd")
		extension = ".zst";
	else if (type == "audio/mpeg")
		extension = ".mp3";
	else if (type == "audio/ogg")
		extension = ".ogg";
	else if (type == "image/avif")
		extension = ".avif";
	else if (type == "image/jpeg")
		extension = ".jpg";
	else if (type == "image/png")
		extension = ".png";
	else if (type == "image/svg+xml")
		extension = ".svg";
	else if (type == "image/tiff")
		extension = ".tif";
	else if (type == "model/obj")
		extension = ".obj";
	else if (type == "multipart/form-data")
		extension = ".form-data";
	else if (type == "text/plain")
		extension = ".txt";
	else if (type == "text/css")
		extension = ".css";
	else if (type == "text/csv")
		extension = ".csv";
	else if (type == "text/html")
		extension = ".html";
	else if (type == "text/javascript")
		extension = ".js";
	else if (type == "text/xml")
		extension = ".xml";
	else
		extension = "";

	return extension;
}

size_t	ContentFetcher::getSizeOfFile(const std::string& filename)
{
	struct stat	path_stat;
	if (stat(filename.c_str(), &path_stat) != 0)
		return 0;
	return path_stat.st_size;
}

void ContentFetcher::serveErrorPage(ClientSocket *client, e_status status)
{
	client->getRequest()->setError(status);
	client->getResponse()->createHTTPHeaders();
	client->setClientState(CLIENT_HAS_FILLED);
}

void ContentFetcher::serveErrorPageBasedOnExistingStatus(ClientSocket *client)
{
	client->getResponse()->createHTTPHeaders();
	client->setClientState(CLIENT_HAS_FILLED);
}

void ContentFetcher::fillResponse(ClientSocket *client)
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
