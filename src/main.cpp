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
	// creating a socket, binding it to an IP address and listening
	ServSockette	ServerSocket(PORT);


	servingLoop(ServerSocket.getSocketFd(), *ServerSocket.getSocketAddr());
	return 0;
}
