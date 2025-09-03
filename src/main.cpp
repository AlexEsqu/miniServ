// Server side C program to demonstrate HTTP Server programming

#include "server.hpp"

int main()
{

	std::string finalResponse;
	std::ifstream input("pages/index.html"); //opening the file as the content for the response
	std::stringstream content;
	content << input.rdbuf();
	
	Response response; //Setting all the necessary infos for the response
	response.setProtocol("HTTP/1.1");
	response.setStatusNum(200);
	response.setContentType("text/html");
	response.setContentLength(content.str().length());
	response.setContent(content.str());
	finalResponse = response.createResponse(); //creating the final response with all the values precedently added

	int server_fd, new_socket;
	long valread;
	struct sockaddr_in address;
	int addrlen = sizeof(address);

	
	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("In socket");
		exit(EXIT_FAILURE);
	}

	// allow socket to be reused and webserv to reload faster wi SO_REUSEADDR
	const int on = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) != 0)
	{
		perror("In socket options");
		exit(EXIT_FAILURE);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);
	memset(address.sin_zero, '\0', sizeof address.sin_zero);

	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		perror("In bind");
		exit(EXIT_FAILURE);
	}

	if (listen(server_fd, 10) < 0)
	{
		perror("In listen");
		exit(EXIT_FAILURE);
	}

	while (1)
	{
		printf("\n+++++++ Waiting for new connection ++++++++\n\n");
		if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
		{
			perror("In accept");
			exit(EXIT_FAILURE);
		}

		char buffer[30000] = {0};
		valread = read( new_socket , buffer, 30000);
		printf("%s\n", buffer);

		Request decodedRequest(buffer);
		Request copyRequest = decodedRequest;

		write(new_socket , finalResponse.c_str() , finalResponse.size());
		printf("------------------Hello message sent-------------------");
		close(new_socket);
	}
	Status code(404);

	std::cout << code.getStatusCode() << " " << code.getStatusMessage() << std::endl;
	return 0;
}
