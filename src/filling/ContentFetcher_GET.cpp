#include "ContentFetcher.hpp"

void ContentFetcher::serveStatic(ClientSocket *client)
{
	std::string fileURL(client->getResponse()->getRoutedURL());

#ifdef DEBUG
	std::cout << "serving static " << fileURL;
#endif

	std::ifstream input(fileURL.c_str(), std::ios::binary);

	if (!input.is_open() || isDirectory(fileURL.c_str()))
	{
		std::cerr << ERROR_FORMAT("Could not open file") << std::endl;
		serveErrorPage(client, NOT_FOUND);
		return;
	}

	client->getResponse()->setContentType(getTypeBasedOnExtension(fileURL));
	size_t size = getSizeOfFile(fileURL);
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

void ContentFetcher::getItemFromServer(ClientSocket *client)
{
	std::cout << "Processing GET request to: " << client->getResponse()->getRoutedURL() << std::endl;

	for (size_t i = 0; i < _executors.size(); i++)
	{
		if (_executors[i]->canExecuteFile(client->getResponse()->getRoutedURL()))
		{
			_executors[i]->executeFile(client);
			client->setClientState(CLIENT_FILLING);
			return;
		}
	}

	std::cout << CGI_FORMAT(" NO CGI ");
	serveStatic(client);
}
