#include "ContentFetcher.hpp"

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

e_dataProgress	ContentFetcher::readCGIChunk(ClientSocket *client)
{
	char buffer[4096];
	ssize_t bytesRead;

	client->updateLastEventTime();

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

	std::string stringBuffer(buffer, bytesRead);
	client->getResponse()->addToContent(stringBuffer);

	return WAITING_FOR_MORE;
}


