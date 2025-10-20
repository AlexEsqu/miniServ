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

void	ContentFetcher::addExecutor(Executor* executor)
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

size_t ContentFetcher::getSizeOfFile(const std::string &filename)
{
	struct stat st;
	if (stat(filename.c_str(), &st) != 0)
	{
		return 0;
	}
	return st.st_size;
}

bool	ContentFetcher::isDirectory(const char *path)
{
	struct stat path_stat;
	if (stat(path, &path_stat) != 0)
		return false;
	return S_ISDIR(path_stat.st_mode);
}

void	ContentFetcher::serveStatic(ClientSocket* client)
{
	std::string		fileURL(client->getResponse()->getRoutedURL());

	#ifdef DEBUG
	std::cout << "serving static " << fileURL;
	#endif

	std::ifstream	input(fileURL.c_str(), std::ios::binary);

	if (!input.is_open() || isDirectory(fileURL.c_str()))
	{
		std::cerr << ERROR_FORMAT("Could not open file") << std::endl;
		serveErrorPage(client, NOT_FOUND);
		return;
	}

	client->getResponse()->setContentType(getTypeBasedOnExtension(fileURL));
	size_t	size = getSizeOfFile(fileURL);
	std::vector<char> buffer(size);
	input.read(buffer.data(), size);
	std::string binaryContent(buffer.begin(), buffer.end());
	client->getResponse()->addToContent(binaryContent);
	client->getResponse()->createHTTPHeaders();
	client->setClientState(CLIENT_HAS_FILLED);
	#ifdef DEBUG
	std::cout << "Filling done\n";
	#endif
}

void	ContentFetcher::serveErrorPage(ClientSocket* client, e_status status)
{
	client->getRequest()->setError(status);
	client->getResponse()->createHTTPHeaders();
	client->setClientState(CLIENT_HAS_FILLED);
}

void	ContentFetcher::serveErrorPageBasedOnExistingStatus(ClientSocket* client)
{
	client->getResponse()->createHTTPHeaders();
	client->setClientState(CLIENT_HAS_FILLED);
}

void	ContentFetcher::getItemFromServer(ClientSocket* client)
{
	std::cout << "Processing GET request to: " << client->getResponse()->getRoutedURL() << std::endl;

	for (size_t i = 0; i < _executors.size(); i++)
	{
		if(_executors[i]->canExecuteFile(client->getResponse()->getRoutedURL()))
		{
			_executors[i]->executeFile(client);
			client->setClientState(CLIENT_FILLING);
			return;
		}
	}

	std::cout << CGI_FORMAT(" NO CGI ");
	serveStatic(client);
}

void ContentFetcher::postItemFromServer(ClientSocket* client)
{
	std::cout << "Processing POST request to: " << client->getResponse()->getRoutedURL() << std::endl;
	parseBody(client);
	handleFileUpload(client);

	// if (request.getResponse()->getRoutedURL().find("upload") != std::string::npos) {
	// 	handleFileUpload(request);
	// } else {
	// 	handleFormSubmission(request);
	// }
}

void	ContentFetcher::handleFormSubmission(ClientSocket* client)
{
	std::string postData = client->getRequest()->getBody();

	std::cout << "POST data received: " << postData << std::endl;

	std::string responseContent =
		"<!DOCTYPE html>"
		"<html><head><title>Form Submitted</title></head>"
		"<body><h1>Form submitted successfully!</h1>"
		"<p>Data received: " + postData + "</p>"
		"<a href='/'>Back to home</a></body></html>";

	client->getResponse()->setContentType("text/html");
	client->getResponse()->addToContent(responseContent.c_str());
	client->getRequest()->setStatus(OK);
}

void	ContentFetcher::parseBody(ClientSocket* client)
{

	std::cout << "body is [" << client->getRequest()->getBody() << "]\n";

	if (client->getRequest()->getContentType().find("application/x-www-form-urlencoded") != std::string::npos)
	{
		parseUrlEncodedBody(client);
	}
	else if (client->getRequest()->getContentType().find("multipart/form-data") != std::string::npos)
	{
		parseMultiPartBody(client);
	}
	else
		client->getRequest()->setError(UNSUPPORTED_MEDIA_TYPE);

	// client->getRequest()->

	// _requestState = PARSING_DONE;
	// return RECEIVED_ALL;
}

// if Content-Type: application/x-www-form-urlencoded
//read key=value&key=value and store data
void	ContentFetcher::parseUrlEncodedBody(ClientSocket* client)
{
	std::string body = client->getRequest()->getBody();
	std::string key = body.substr(0, body.find("="));
	std::string value = body.substr(key.size() + 1, body.find("&"));
	std::cout << GREEN  << key << " = " << value << STOP_COLOR << std::endl;

	while(1)
	{
		key = body.substr(0, body.find("="));
		value = body.substr(key.size() + 1, body.find("&"));
	}

}

// if Content-Type: multipart/form-data; boundary=---------------------------84751486837113120871083762733
// store boundary and read each section until boundary and store data
void	ContentFetcher::parseMultiPartBody(ClientSocket* )
{
 
}

void	ContentFetcher::handleFileUpload(ClientSocket* client)
{
	std::cout << "upload directory: [" << client->getRequest()->getRoute()->getUploadDirectory() << "]\n";
	std::cout << "request URL: [" << client->getRequest()->getRequestedURL() << "]\n";

	std::string	uploadPath = client->getRequest()->getRoute()->getUploadDirectory();
	uploadPath.append(client->getRequest()->getRequestedURL());

	std::cout << uploadPath << "\n";

	FileHandler	upload(uploadPath);

	upload.writeToFile(client->getRequest()->getBody());

	std::string uploadResponse =
		"<!DOCTYPE html>"
		"<html><head><title>Upload Complete</title></head>"
		"<body><h1>File uploaded successfully!</h1>"
		"<a href='/'>Back to home</a></body></html>";
	client->getResponse()->setContentType("text/html");
	client->getResponse()->addToContent(uploadResponse.c_str());
	client->getRequest()->setStatus(CREATED);
	client->getResponse()->createHTTPHeaders();
	client->setClientState(CLIENT_HAS_FILLED);
}

void	ContentFetcher::deleteItemFromServer(ClientSocket*  )
{
	std::cout << "DELETE is not yet implemented\n";
}

void	ContentFetcher::fillResponse(ClientSocket* client)
{
	// create a response object and attach it to the request
	client->createNewResponse();

	Request*	request = client->getRequest();

	#ifdef DEBUG
	std::cout << "Filling request to: " << client->getResponse()->getRoutedURL() << std::endl;
	#endif

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

e_dataProgress	ContentFetcher::readCGIChunk(ClientSocket* client)
{
	char buffer[4096];
	ssize_t bytesRead;

	bytesRead = read(client->getCgiPipeFd(), buffer, sizeof(buffer));

	// If there is nothing the read in the buffer, reached the end of the CGI output
	if (bytesRead == 0)
	{
		client->stopReadingPipe();
		// wrap response content / error page with HTTP headers
		client->getResponse()->createHTTPHeaders();
		client->setClientState(CLIENT_HAS_FILLED);
		return RECEIVED_ALL;
	}
	// Encountered a read error
	if (bytesRead < 0)
	{
		if (errno != EAGAIN && errno != EWOULDBLOCK)
		{
			std::cout << "nothing in pipe\n";
			return WAITING_FOR_MORE;
		}
		else
		{
			perror("read from CGI pipe failed");
			throw std::runtime_error("Failed to read CGI output");
		}
	}

	std::string	stringBuffer(buffer, bytesRead);
	client->getResponse()->addToContent(stringBuffer);

	return WAITING_FOR_MORE;
}
