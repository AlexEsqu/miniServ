#include "ContentFetcher.hpp"

//----------------- CONSTRUCTORS ---------------------//

ContentFetcher::ContentFetcher()
{

}

ContentFetcher::ContentFetcher(const ContentFetcher &original)
{
	*this = original;
}

ContentFetcher& ContentFetcher::operator=(const ContentFetcher &original)
{
	if (this != &original)
		executors = original.executors;
	return *this;
}

//----------------- DESTRUCTOR -----------------------//

ContentFetcher::~ContentFetcher()
{
	for (std::vector<Executor*>::iterator item = executors.begin(); item != executors.end(); item++)
		delete *item;
}

//------------------- MEMBER FUNCTIONS ------------------------//

std::string	ContentFetcher::getTypeBasedOnExtension(const std::string& filePath)
{
	size_t dotPos = filePath.find_last_of('.');
	if (dotPos == std::string::npos)
		return "text/plain";

	std::string extension = filePath.substr(dotPos + 1);

	std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
	if (extension == "html" || extension == "htm") return "text/html";
	if (extension == "css") return "text/css";
	if (extension == "js") return "application/javascript";
	if (extension == "jpg" || extension == "jpeg") return "image/jpeg";
	if (extension == "png") return "image/png";
	if (extension == "gif") return "image/gif";
	if (extension == "svg") return "image/svg+xml";
	if (extension == "ico") return "image/x-icon";
	if (extension == "txt") return "text/plain";
	if (extension == "pdf") return "application/pdf";
	if (extension == "webp") return "image/wepb";
	if (extension == "png") return "image/png";

	return "text/plain";
}

size_t	ContentFetcher::getSizeOfFile(const std::string& filename) {
	struct stat st;
	if(stat(filename.c_str(), &st) != 0) {
		return 0;
	}
	return st.st_size;
}

bool	ContentFetcher::isDirectory(const char *path)
{
	struct stat path_stat;
	stat(path, &path_stat);
	return S_ISDIR(path_stat.st_mode);
}

void	ContentFetcher::serveStatic(Response& response)
{
	std::string			fileURL(response.getRoutedURL());

	std::ifstream		input(fileURL.c_str(), std::ios::binary);

	if (!input.is_open() || isDirectory(fileURL.c_str()))
	{
		std::cerr << ERROR_FORMAT("Could not open file") << std::endl;
		response.setStatusNum(404);
		response.setHTTPResponse();
		return;
	}

	response.setContentType(getTypeBasedOnExtension(fileURL));
	size_t	size = getSizeOfFile(fileURL);
	std::vector<char> buffer(size);
	input.read(buffer.data(), size);
	response.setContent(buffer);
}

void ContentFetcher::executeIfCGI(Response& response)
{
	for (size_t i = 0; i < executors.size(); i++)
	{
		if(executors[i]->canExecuteFile(response))
			return executors[i]->executeFile(response);
	}

	std::cout << CGI_FORMAT(" NO CGI ");
	return (serveStatic(response));
}

void	ContentFetcher::addExecutor(Executor* executor)
{
	executors.push_back(executor);
}

void	ContentFetcher::fillContent(Response& response)
{
	if (response.getRequest()->getMethod() == "GET")
		executeIfCGI(response);


	response.setHTTPResponse();
}

void	ContentFetcher::craftSendHTTPResponse(ClientSocket* client)
{
	Response response(client->getRequest());
	fillContent(response);
	#ifdef DEBUG
		std::cout << response.getStatus() << std::endl;
	#endif

	if (write(client->getSocketFd(),
		response.getHTTPResponse().c_str(),
		response.getHTTPResponse().size()) < 0)
		throw std::runtime_error("write fail");
	std::cout << VALID_FORMAT("\n++++++++ Answer has been sent ++++++++ \n");

	if (client->getRequest()->isKeepAlive()) {
		client->resetRequest();
	}

	else {
		// epoll_ctl(_epollFd, EPOLL_CTL_DEL, Connecting->getSocketFd(), NULL);
		// close(Connecting->getSocketFd());
		delete client;
		return ;
	}
}


