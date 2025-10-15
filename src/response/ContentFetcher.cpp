#include "ContentFetcher.hpp"

//----------------- CONSTRUCTORS ---------------------//

ContentFetcher::ContentFetcher()
{
}

ContentFetcher::ContentFetcher(const ContentFetcher &original)
{
	*this = original;
}

ContentFetcher &ContentFetcher::operator=(const ContentFetcher &original)
{
	if (this != &original)
		executors = original.executors;
	return *this;
}

//----------------- DESTRUCTOR -----------------------//

ContentFetcher::~ContentFetcher()
{
	for (std::vector<Executor *>::iterator item = executors.begin(); item != executors.end(); item++)
		delete *item;
}

//----------------- INTERNAL ---------------------//

void	ContentFetcher::addExecutor(Executor* executor)
{
	executors.push_back(executor);
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

bool ContentFetcher::isDirectory(const char *path)
{
	struct stat path_stat;
	stat(path, &path_stat);
	return S_ISDIR(path_stat.st_mode);
}

void	ContentFetcher::serveStatic(Request& request)
{
	std::string			fileURL(request.getResponse()->getRoutedURL());

	std::ifstream input(fileURL.c_str(), std::ios::binary);

	if (!input.is_open() || isDirectory(fileURL.c_str()))
	{
		std::cerr << ERROR_FORMAT("Could not open file") << std::endl;
		request.getResponse()->setStatusNum(404);
		request.getResponse()->createHTTPHeaders();
		return;
	}

	request.getResponse()->setContentType(getTypeBasedOnExtension(fileURL));
	size_t	size = getSizeOfFile(fileURL);
	std::vector<char> buffer(size);
	input.read(buffer.data(), size);
	std::string binaryContent(buffer.begin(), buffer.end());
	request.getResponse()->addToContent(binaryContent);
}

void ContentFetcher::getItemFromServer(Request& request)
{
	for (size_t i = 0; i < executors.size(); i++)
	{
		if(executors[i]->canExecuteFile(request.getResponse()->getRoutedURL()))
			return executors[i]->executeFile(request);
	}

	std::cout << CGI_FORMAT(" NO CGI ");
	serveStatic(request);
}

void ContentFetcher::postItemFromServer(Request& request)
{
	std::cout << "Processing POST request to: " << request.getResponse()->getRoutedURL() << std::endl;

	handleFileUpload(request);

	// if (request.getResponse()->getRoutedURL().find("upload") != std::string::npos) {
	// 	handleFileUpload(request);
	// } else {
	// 	handleFormSubmission(request);
	// }
}

void	ContentFetcher::handleFormSubmission(Request& request)
{
	std::string postData = request.getBody();

	std::cout << "POST data received: " << postData << std::endl;

	std::string responseContent =
		"<!DOCTYPE html>"
		"<html><head><title>Form Submitted</title></head>"
		"<body><h1>Form submitted successfully!</h1>"
		"<p>Data received: " + postData + "</p>"
		"<a href='/'>Back to home</a></body></html>";

	request.getResponse()->setContentType("text/html");
	request.getResponse()->addToContent(responseContent.c_str());
	request.getResponse()->setStatusNum(200);
}

void	ContentFetcher::handleFileUpload(Request& request)
{
	FileHandler	upload(request.getResponse()->getRoutedURL());

	upload.writeToFile(request.getBody());

	std::string uploadResponse =
		"<!DOCTYPE html>"
		"<html><head><title>Upload Complete</title></head>"
		"<body><h1>File uploaded successfully!</h1>"
		"<a href='/'>Back to home</a></body></html>";

	request.getResponse()->setContentType("text/html");
	request.getResponse()->addToContent(uploadResponse.c_str());
	request.getResponse()->setStatusNum(201);
}

void	ContentFetcher::deleteItemFromServer(Request& )
{
	std::cout << "DELETE is not yet implemented\n";
}


void	ContentFetcher::fillResponse(Request& request)
{
	// create a response object in the request, with routed url
	createResponseToFill(&request);

	if (request.getMethod() == "GET")
		getItemFromServer(request);
	if (request.getMethod() == "POST")
		postItemFromServer(request);
	if (request.getMethod() == "DELETE")
		deleteItemFromServer(request);
	request.setParsingState(FILLING_DONE);

	// creating HTTP headers wrapping the response (needs to be at the end to have Content Size)
	request.getResponse()->createHTTPHeaders();
}


Response	ContentFetcher::createResponseToFill(Request* request)
{
	request->setResponse(new Response);

	try
	{
		request->getResponse()->setStatusNum(request->getStatus().getStatusCode());
		request->getResponse()->setRequest(request);
		request->getResponse()->setRoutedUrl(request->getRequestedURL());
	}

	catch (const HTTPError &e)
	{
		std::cout << "error in fetcher\n";
		std::cout << e.what() << "\n";
	}

	return (*request->getResponse());
}
