#include "ContentFetcher.hpp"

void ContentFetcher::getItemFromServer(ClientSocket *client)
{
	verboseLog("Processing GET request to: " + client->getResponse().getRoutedURL());

	for (size_t i = 0; i < _executors.size(); i++)
	{
		if (_executors[i]->canExecuteFile(client->getResponse().getRoutedURL()))
		{
			std::cout << CGI_FORMAT(" CGI ") << client->getResponse().getRoutedURL();
			_executors[i]->executeFile(client);
			client->setClientState(CLIENT_FILLING);
			return;
		}
	}

	serveStatic(client);
}

std::string ContentFetcher::findFileInDirectory(std::string directory, std::string filename) // EX "/upload", "picture" => picture.jpeg
{
	DIR					*dir;
	struct dirent		*ent;
	if ((dir = opendir(directory.c_str())) != NULL)
	{
		while ((ent = readdir(dir)) != NULL)
		{
			char		*entChar = ent->d_name;
			std::string	ent(entChar);

			size_t		dotPos = ent.find(".");
			if (dotPos != std::string::npos)
			{
				std::string		fileWithoutExtension = ent.substr(0, dotPos);
				if (fileWithoutExtension == filename)
				{
					std::string	filePath = directory + "/" + ent;
					return filePath;
				}
			}
		}
		closedir(dir);
	}
	else
	{
		std::cerr << ERROR_FORMAT("Could not open directory") << std::endl;
		return std::string();
	}
	return std::string();
}

void ContentFetcher::serveStatic(ClientSocket *client)
{
	std::string		fileURL(client->getResponse().getRoutedURL());

	size_t			filenamePos = fileURL.find_last_of('/');
	std::string		filename;
	if (filenamePos != std::string::npos)
		filename = fileURL.substr(filenamePos);
	std::ifstream	input(fileURL.c_str(), std::ios::binary);

	if (!input.is_open() || Router::isDirectory(fileURL.c_str())) // if it has no extension, try to find the full filename in the directory (is still in testing)
	{
		std::cerr << MAGENTA << "Found the file in the directory: " << findFileInDirectory(client->getRequest().getRoute()->getUploadDirectory(),filename ) << STOP_COLOR << std::endl;
		//then try to open findFileInDirectory
	}

	// if the file is a directory and not routed to a default file
	// serve auto index instead of static page
	if (Router::isDirectory(fileURL.c_str()))
		return serveDirectoryListing(client, fileURL);
	client->getResponse().setContentType(getTypeBasedOnExtension(fileURL));
	size_t size = getSizeOfFile(fileURL);
	std::vector<char> buffer(size);
	input.read(buffer.data(), size);
	std::string binaryContent(buffer.begin(), buffer.end());
	client->getResponse().addToContent(binaryContent);
	client->getResponse().createHTTPHeaders();
	client->setClientState(CLIENT_HAS_FILLED);
	verboseLog("Filling done");
}

void	ContentFetcher::addCgiResultToResponse(Response &response, Buffer& buffer)
{
	std::string	allContent = buffer.getAllContent();

	// annoyingly, cgi provides http headers to have fun with
	size_t headerEnd = allContent.find("\n");
	if (headerEnd != std::string::npos)
	{
		std::string headers = allContent.substr(0, headerEnd);
		trim(headers);
		std::string body = allContent.substr(headerEnd + 1);

		// std::cout << "\nparsing headers [" << headers << "]" << std::endl;
		parseCgiHeader(response, headers);
		response.addToContent(body);
	}
	else
		response.addToContent(allContent);
}

e_dataProgress ContentFetcher::readCGIChunk(ClientSocket *client)
{
	char buffer[4096];
	ssize_t bytesRead;

	bytesRead = read(client->getCgiPipeFd(), buffer, sizeof(buffer));

	// If there is nothing the read in the buffer, reached the end of the CGI output
	if (bytesRead == 0)
	{
		client->stopReadingPipe();

		// parsing the CGI response into headers and body
		addCgiResultToResponse(client->getResponse(), client->getCgiBuffer());

		// cleaning up the buffer (try commenting it for the clock.py script, is cute)
		client->getCgiBuffer().clearBuffer();

		// wrapping in headers and signaling the CGI is complete !
		client->getResponse().createHTTPHeaders();
		client->setClientState(CLIENT_HAS_FILLED);
		return RECEIVED_ALL;
	}
	// Encountered a read error
	if (bytesRead < 0)
	{
		if (errno != EAGAIN && errno != EWOULDBLOCK)
		{
			verboseLog("nothing in pipe");
			return WAITING_FOR_MORE;
		}
		else
		{
			perror("read from CGI pipe failed");
			throw std::runtime_error("Failed to read CGI output");
		}
	}

	// writing to a buffer cuz there may be headers to parse later on
	// could be optimized by having a named pipe and reading from it twice?
	client->getCgiBuffer().writeToBuffer(buffer, bytesRead);

	return WAITING_FOR_MORE;
}

void	ContentFetcher::serveDirectoryListing(ClientSocket* client, std::string& fileURL)
{
	// Gets the route to check if autoindex is enabled
	const Route* route = client->getRequest().getRoute();

	std::cout << "is autoindex = " << route->isAutoIndex() << "\n";
	if (route && route->isAutoIndex())
	{
		std::string requestUri = client->getRequest().getRequestedURL();
		std::string directoryListingPage = createDirectoryListing(fileURL, requestUri);

		if (!directoryListingPage.empty())
		{
			client->getResponse().setContentType("text/html");
			client->getResponse().addToContent(directoryListingPage);
			client->getResponse().createHTTPHeaders();
			client->setClientState(CLIENT_HAS_FILLED);
			return;
		}
	}
	verboseLog("Directory listing not allowed or failed");
	serveErrorPage(client, NOT_FOUND);
	return;
}

std::string ContentFetcher::createDirectoryListing(const std::string& path, const std::string& requestUri)
{
	DIR*						dir;
	struct dirent*				entry;
	std::stringstream			html;
	std::vector<std::string>	entries;

	if ((dir = opendir(path.c_str())) == NULL)
		return "";

	// reads directory entries
	while ((entry = readdir(dir)) != NULL)
		entries.push_back(entry->d_name);

	// sorts entries alphabetically
	std::sort(entries.begin(), entries.end());

	// Generate HTML
	html << "<!DOCTYPE html>\n"
		<< "<html>\n"
		<< "<head>\n"
		<< "	<title>Index of " << requestUri << "</title>\n"
		<< "	<style>\n"
		<< "		body { font-family: Arial, sans-serif; margin: 20px; }\n"
		<< "		h1 { border-bottom: 1px solid #ccc; padding-bottom: 10px; }\n"
		<< "		table { width: 100%; border-collapse: collapse; }\n"
		<< "		th, td { text-align: left; padding: 8px; }\n"
		<< "		tr:nth-child(even) { background-color: #f2f2f2; }\n"
		<< "		a { text-decoration: none; color: #0366d6; }\n"
		<< "		a:hover { text-decoration: underline; }\n"
		<< "		.size { text-align: right; }\n"
		<< "		.date { color: #6a737d; }\n"
		<< "	</style>\n"
		<< "</head>\n"
		<< "<body>\n"
		<< "	<h1>Index of " << requestUri << "</h1>\n"
		<< "	<table>\n"
		<< "		<tr><th>Name</th><th>Last Modified</th><th>Size</th></tr>\n";

	// Parent directory link (except for root)
	if (requestUri != "/")
		html << "		<tr><td><a href=\"..\">..</a></td><td></td><td></td></tr>\n";

	for (size_t i = 0; i < entries.size(); i++)
	{
		const std::string& name = entries[i];
		// skips current directory and parent entry
		if (name == "." || (name == ".." && requestUri != "/"))
			continue;

		std::string fullPath = path + "/" + name;
		struct stat statbuf;

		if (stat(fullPath.c_str(), &statbuf) == 0)
		{
			// Format file size
			std::string size;
			if (S_ISDIR(statbuf.st_mode))
				size = "-";
			else
			{
				std::stringstream sizeStream;
				if (statbuf.st_size < 1024)
					sizeStream << statbuf.st_size << " B";
				else if (statbuf.st_size < 1024 * 1024)
					sizeStream << std::fixed << std::setprecision(1) << (float)statbuf.st_size / 1024 << " KB";
				else
					sizeStream << std::fixed << std::setprecision(1) << (float)statbuf.st_size / (1024 * 1024) << " MB";
				size = sizeStream.str();
			}

			// Format last modified time
			char timeStr[100];
			strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M", localtime(&statbuf.st_mtime));

			// Entry is a directory, add trailing slash
			std::string displayName = name;
			std::string linkName = name;
			if (S_ISDIR(statbuf.st_mode))
				displayName += "/";

			html << "		<tr>"
				<< "<td><a href=\"" << linkName << (S_ISDIR(statbuf.st_mode) ? "/" : "") << "\">" << displayName << "</a></td>"
				<< "<td class=\"date\">" << timeStr << "</td>"
				<< "<td class=\"size\">" << size << "</td>"
				<< "</tr>\n";
		}
	}

	html << "	</table>\n"
		<< "	<hr>\n"
		<< "	<p>miniServ - " << std::time(NULL) << "</p>\n"
		<< "</body>\n"
		<< "</html>";

	closedir(dir);
	return html.str();
}

// Same as the getItemFromServer but with no body
void ContentFetcher::headItemFromServer(ClientSocket *client)
{
	verboseLog("Processing HEAD request to: " + client->getResponse().getRoutedURL());

	std::string path = Router::routeFilePathForGet(client->getRequest().getRequestedURL(), client->getRequest());
	if (path.empty() || !Router::isExisting(path.c_str()) || Router::isDirectory(path.c_str()))
	{
		serveErrorPage(client, NOT_FOUND);
		return;
	}

	client->getResponse().setContentType(getTypeBasedOnExtension(path));
	client->getResponse().createHTTPHeaders();
	client->setClientState(CLIENT_HAS_FILLED);
	verboseLog("HEAD prepared for: " + path);
}
