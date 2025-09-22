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

bool	ContentFetcher::isDirectory(const char *path)
{
	struct stat path_stat;
	stat(path, &path_stat);
	return S_ISDIR(path_stat.st_mode);
}

void	ContentFetcher::serveStatic(Response& response)
{
	std::ifstream input(response.getRoutedURL().c_str()); // opening the file as the content for the response
	std::stringstream content;

	if (!input.is_open() || isDirectory(response.getRoutedURL().c_str()))
	{
		std::cerr << ERROR_FORMAT("Could not open file") << std::endl;
		response.setStatusNum(404);
		response.setHTTPResponse();
		return;
	}
	content << input.rdbuf();
	response.setContent(content.str());
}

void ContentFetcher::executeIfCGI(Response& response)
{
	for (size_t i = 0; i < executors.size(); i++)
	{
		if(executors[i]->canExecuteFile(response))
			return executors[i]->executeFile(response);
	}
	
	std::cout << CGI_FORMAT("NO CGI APPLICABLE") << std::endl;
	return (serveStatic(response));
}

void	ContentFetcher::addExecutor(Executor* executor)
{
	executors.push_back(executor);
}

void	ContentFetcher::fillContent(Response& response)
{
	executeIfCGI(response);
	response.setHTTPResponse();
}


