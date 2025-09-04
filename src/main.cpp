// Server side C program to demonstrate HTTP Server programming

#include "server.hpp"

std::string createResponse(std::string filePath)
{
	std::string finalResponse;
	std::ifstream input(filePath.c_str()); // opening the file as the content for the response
	std::stringstream content;
	content << input.rdbuf(); // putting the content of the input file into the content variable

	Response response; // Setting all the necessary infos for the response
	response.setProtocol("HTTP/1.1");
	response.setStatusNum(200);
	response.setContentType("text/html");
	response.setContentLength(content.str().length());
	response.setContent(content.str());
	finalResponse = response.createResponse(); // creating the final response with all the values precedently added
	return (finalResponse);
}

// Binding a socket using a standard IP address options (AF_INET, INADDR_ANY)
struct sockaddr_in bindSocketToIPAddress(int socket_fd)
{
	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);
	memset(address.sin_zero, '\0', sizeof address.sin_zero);

	if (bind(socket_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		perror("In bind");
		exit(EXIT_FAILURE);
	}

	if (listen(socket_fd, 10) < 0)
	{
		perror("In listen");
		exit(EXIT_FAILURE);
	}

	return (address);
}

void	servingLoop(int server_fd, struct sockaddr_in address)
{
	int addrlen = sizeof(address);
	int new_socket;
	long valread;

	while (1)
	{
		printf("\n+++++++ Waiting for new connection ++++++++\n\n");
		if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
		{
			perror("In accept");
			exit(EXIT_FAILURE);
		}

		char buffer[30000] = {0};
		valread = read(new_socket, buffer, 30000);
		printf("%s\n", buffer);

		Request decodedRequest(buffer);
		Request copyRequest = decodedRequest;

		std::string finalResponse = createResponse("pages/index.html");

		write(new_socket, finalResponse.c_str(), finalResponse.size());
		printf("------------------Hello message sent-------------------");
		close(new_socket);
	}
}


int main()
{
	Sockette	ServerSocket;
	bindSocketToIPAddress(ServerSocket.getSocketFd());
	servingLoop(ServerSocket.getSocketFd(), ServerSocket.getSocketAddr());
	return 0;
}
