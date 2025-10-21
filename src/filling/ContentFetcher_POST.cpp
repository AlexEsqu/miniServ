#include "ContentFetcher.hpp"

void ContentFetcher::postItemFromServer(ClientSocket *client)
{
	std::cout << "Processing POST request to: " << client->getResponse()->getRoutedURL() << std::endl;

	parseBody(client);

	createPostResponsePage(client);
}

void ContentFetcher::handleFormSubmission(ClientSocket *client)
{
	std::string postData = client->getRequest()->getBody();

	std::cout << "POST data received: " << postData << std::endl;

	std::string responseContent =
		"<!DOCTYPE html>"
		"<html><head><title>Form Submitted</title></head>"
		"<body><h1>Form submitted successfully!</h1>"
		"<p>Data received: " +
		postData + "</p>"
				   "<a href='/'>Back to home</a></body></html>";

	client->getResponse()->setContentType("text/html");
	client->getResponse()->addToContent(responseContent.c_str());
	client->getRequest()->setStatus(OK);
}

void ContentFetcher::parseBody(ClientSocket *client)
{

	std::cout << "body is [\n" << client->getRequest()->getBody() << "]\n";

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
}

std::string	ContentFetcher::findUploadFilepath(const Route *route, const std::string &uri)
{
	std::string uploadFilepath = uri;
	std::string uploadDirectory = route->getUploadDirectory();
	std::string routeDirectory = route->getURLPath();
	uploadFilepath = uploadFilepath.replace(0, routeDirectory.size(), uploadDirectory);
	std::cout << "upload path: " << uploadFilepath << std::endl;

	return (uploadFilepath);
}

// if Content-Type: application/x-www-form-urlencoded
// read key=value&key=value and store data
void		ContentFetcher::parseUrlEncodedBody(ClientSocket *client)
{
	size_t i = 0;
	std::string pathToUploadDirectory = findUploadFilepath(client->getRequest()->getRoute(), client->getRequest()->getRequestedURL());
	std::string body = client->getRequest()->getBody();

	while (1)
	{
		std::cout << "finding = : " << body.find("=", i) << "\n";
		std::string key = body.substr(i, body.find("=", i) - i);
		i += key.size() + 1;
		std::string value = body.substr(i, body.find("&") - i);
		i += value.size() + 1;
		std::cout << GREEN << key << " = " << value << STOP_COLOR << std::endl;

		// create file with the name key, put value in it
		std::string	pathToUploadFile = pathToUploadDirectory + "/" + key;
		FileHandler file(pathToUploadFile);
		file.writeToFile(value);
		if (i > body.length())
			break;
	}
}

// if Content-Type: multipart/form-data; boundary=---------------------------84751486837113120871083762733
// store boundary and read each section until boundary and store data
void ContentFetcher::parseMultiPartBody(ClientSocket *client)
{
	client->getResponse()->setBoundary();

	/*
	------------------------6d965394600a1b0d
	--------------------------6d965394600a1b0d
	--------------------------6d965394600a1b0d--
	*/
}

void ContentFetcher::createPostResponsePage(ClientSocket *client)
{
	std::string uploadResponse =
		"<!DOCTYPE html>"
		"<html><head><title>Upload Complete</title></head>"
		"<body><h1>File uploaded successfully!</h1>"
		"<a href='/'>Back to home</a></body></html>";

	client->getResponse()->setContentType("text/html");
	//client->getResponse()->addToContent(uploadResponse.c_str());
	client->getRequest()->setStatus(CREATED);
	client->getResponse()->createHTTPHeaders();
	client->setClientState(CLIENT_HAS_FILLED);
}
