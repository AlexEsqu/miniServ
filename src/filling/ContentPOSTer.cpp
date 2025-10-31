#include "ContentFetcher.hpp"

void ContentFetcher::postItemFromServer(ClientSocket *client)
{
	verboseLog("Processing POST request to: " + client->getResponse().getRoutedURL());

	parseBodyDataAndUpload(client);

	createPostResponsePage(client);
}

void ContentFetcher::parseBodyDataAndUpload(ClientSocket *client)
{
	if (client->getRequest().getContentType().find("application/x-www-form-urlencoded") != std::string::npos)
	{
		parseUrlEncodedBody(client);
	}
	else if (client->getRequest().getContentType().find("multipart/form-data") != std::string::npos)
	{
		parseMultiPartBody(client);
	}
	else if (client->getRequest().getContentType().find("text/plain") != std::string::npos
		|| client->getRequest().getContentType().find("plain/text") != std::string::npos)
		parsePlainBody(client);
	else
		client->getRequest().setError(UNSUPPORTED_MEDIA_TYPE);
}

std::string ContentFetcher::extractBoundary(const std::string &contentType)
{
	return (contentType.substr(contentType.find("=") + 1, contentType.size()));
}

// if Content-Type: application/x-www-form-urlencoded
// read key=value&key=value and store data
// putting body in memroy since assuming urlencoded are short key values
void ContentFetcher::parseUrlEncodedBody(ClientSocket *client)
{
	size_t i = 0;
	std::string body = client->getRequest().getBody();

	while (1)
	{
		std::cout << "finding = : " << body.find("=", i) << "\n";
		std::string key = body.substr(i, body.find("=", i) - i);
		i += key.size() + 1;
		std::string value = body.substr(i, body.find("&") - i);
		i += value.size() + 1;
		std::cout << GREEN << key << " = " << value << STOP_COLOR << std::endl;

		// create file with the name key, put value in it
		std::string pathToUploadFile = client->getResponse().getRoutedURL() + "/" + key;
		FileHandler file(pathToUploadFile);
		file.writeToFile(value);
		if (i > body.length())
			break;
	}
}

void extractMultiPartHeaderBlock(std::istream &bodyReader, std::map<std::string, std::string> &multiPartHeaderMap, std::string &line)
{
	while (1)
	{
		if (line.empty() || line == "\r")
			break;
		size_t colonPos = line.find(":");
		if (colonPos != std::string::npos)
		{
			std::string headerName = line.substr(0, colonPos);
			std::string headerValue = line.substr(colonPos + 1, line.size() - 1); // removing larst \r
			multiPartHeaderMap[headerName] = trim(headerValue);
		}
		std::getline(bodyReader, line);
	}
}

std::string generateFilename(ClientSocket *client, std::istream &bodyReader, std::string line)
{
	std::map<std::string, std::string> multiPartHeaderMap; // not stored past loop for now, bad idea ?
	extractMultiPartHeaderBlock(bodyReader, multiPartHeaderMap, line);

	// trying to find a name for the posted result
	std::string disposition = multiPartHeaderMap["Content-Disposition"];
	std::string uploadFilePath = client->getRequest().getRoute()->getUploadDirectory() + "/" + client->getRequest().getStringSessionId() + "_" ;
	size_t filenamePos = disposition.find("filename=\"");
	size_t namePos = disposition.find("name=\"");
	std::string filename;
	std::string extension;
	if (filenamePos != std::string::npos) // getting the filename extension to append it later to the name
	{
		size_t filenameEnd = disposition.find("\"", filenamePos + 10);
		filename = disposition.substr(filenamePos + 10, filenameEnd - (filenamePos + 10));
		size_t extensionPos = filename.find_last_of('.');
		extension = filename.substr(extensionPos);
		filename = "";
	}
	if (namePos != std::string::npos) // otherwise if no filename was provided, just get the name as name of file
	{
		namePos = disposition.find("name=\"");
		size_t filenameEnd = disposition.find("\"", namePos + 6);
		filename = disposition.substr(namePos + 6, filenameEnd - (namePos + 6));
	}
	else
	{
		client->getResponse().setError(BAD_REQUEST);
		return("");
	}

	uploadFilePath += filename;
	if (extension != "")
		uploadFilePath += extension;
	return(uploadFilePath);
}

// if Content-Type: multipart/form-data; boundary=---------------------------84751486837113120871083762733
// store boundary and read each section until boundary and store data
// trying to read from buffer file since multipart may contain images or heavy files
void ContentFetcher::parseMultiPartBody(ClientSocket *client)
{
	if (client->getRequest().getContentType().empty())
	{
		client->getRequest().setError(BAD_REQUEST);
		return;
	}

	std::string boundary = extractBoundary(client->getRequest().getContentType());
	std::istream &bodyReader = client->getRequest().getStreamFromBodyBuffer();

	// if the boundary has not been extracted, should trigger here
	if (client->getRequest().hasError())
	{
		client->getRequest().setError(BAD_REQUEST);
		return;
	}

	std::string line;
	e_mutipartState parsingState = MP_STARTING_LINE;
	while (std::getline(bodyReader, line))
	{
		trim(line);

		// the first line must contain the boundary
		if (parsingState == MP_STARTING_LINE)
		{
			// wrong syntax if first line of a block has no boundary
			if (line.find(boundary) == std::string::npos)
			{
				client->getResponse().setError(BAD_REQUEST);
				return;
			}
			// if line has boundary + "--" it's the end of multipart body
			if (line == boundary + "--")
				return;

			// switching to next step
			parsingState = MP_HEADERS;
			continue;
		}

		// there can be multiple headers, to be parsed into a header map
		if (parsingState == MP_HEADERS)
		{

			// create the file
			FileHandler multiPartBlock(generateFilename(client, bodyReader,line));

			// read the damn file
			// without getline to avoid corruption by removal of random \n
			while (std::getline(bodyReader, line))
			{
				// Stop reading if we encounter the next boundary
				if (line.find(boundary) != std::string::npos)
				{
					if (line == boundary + "--\r")
						return;
					break;
				}
				line.append("\n");

				multiPartBlock.writeToFile(line);
			}
		}
	}

	// the last line should contain the boundary
	if (line == client->getResponse().getBoundary() + "--")
	{
		client->getResponse().setError(BAD_REQUEST);
	}
}

void ContentFetcher::parsePlainBody(ClientSocket *client)
{
	std::istream &bodyReader = client->getRequest().getStreamFromBodyBuffer();
	if (!bodyReader)
	{
		client->getRequest().setError(INTERNAL_SERVER_ERROR);
		return;
	}

	std::string uploadPath = client->getResponse().getRoutedURL();
	if (Router::isDirectory(uploadPath))
	{
		uploadPath = FileHandler::generateRandomFileName(uploadPath + "/") + ".txt";
	}

	if (uploadPath.empty())
		client->getRequest().setError(NOT_FOUND);
	else if (!Router::isAllowedWrite(uploadPath.c_str()))
		client->getRequest().setError(FORBIDDEN);
	if (client->getRequest().hasError())
		return;

	// Write body as-is to file
	try
	{
		FileHandler fh(uploadPath);
		fh.writeToFile(client->getRequest().getBody());
	}
	catch (...)
	{
		client->getRequest().setError(INTERNAL_SERVER_ERROR);
		return;
	}
}

void ContentFetcher::createPostResponsePage(ClientSocket *client)
{
	std::string uploadResponse =
		"<!DOCTYPE html>"
		"<html><head><title>Upload Complete</title></head>"
		"<body><h1>File uploaded successfully!</h1>"
		"<a href='/'>Back to home</a></body></html>";

	client->getResponse().setContentType("text/html");
	// client->getResponse().addToContent(uploadResponse.c_str());
	client->getRequest().setStatus(CREATED);
	client->getResponse().createHTTPHeaders();
	client->setClientState(CLIENT_HAS_FILLED);
}
