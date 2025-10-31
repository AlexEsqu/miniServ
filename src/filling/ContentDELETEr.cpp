#include "ContentFetcher.hpp"

void ContentFetcher::deleteItemFromServer(ClientSocket *client)
{
	verboseLog("Processing DELETE request to: " + client->getResponse().getRoutedURL());

	std::string path = Router::routeFilePathForGet(client->getRequest().getRequestedURL(), client->getRequest());
	if (path.empty() || !Router::isExisting(path.c_str()))
	{
		serveErrorPage(client, NOT_FOUND);
		return;
	}

	// NOT AUTHORIZED page if not allowed


	// error if trying to delete a directory
	if (Router::isDirectory(path.c_str()))
	{
		serveErrorPage(client, FORBIDDEN);
		return;
	}

	else
	{
		if (std::remove(path.c_str()) != 0)
		{
			serveErrorPage(client, INTERNAL_SERVER_ERROR);
			return;
		}
	}

	client->getResponse().setStatus(NO_CONTENT);
	client->getResponse().createHTTPHeaders();
	client->setClientState(CLIENT_HAS_FILLED);
	verboseLog("DELETE succeeded for: " + path);
}
