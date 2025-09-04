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

void	servingLoop(Sockette &ServSocket)
{
	long valread;

	while (1)
	{
		printf("\n+++++++ Waiting for new connection ++++++++\n\n");
		SocketteAccept Worker(ServSocket);

		char buffer[30000] = {0};
		valread = read(Worker.getSocketFd(), buffer, 30000);
		printf("%s\n", buffer);

		Request decodedRequest(buffer);
		Request copyRequest = decodedRequest;

		std::string finalResponse = createResponse("pages/index.html");

		write(Worker.getSocketFd(), finalResponse.c_str(), finalResponse.size());
		printf("------------------Hello message sent-------------------");
		close(Worker.getSocketFd());
	}
}


int main()
{
	// creating a socket, binding it to an IP address and listening
	SocketteListen	ServerSocket(PORT);


	servingLoop(ServerSocket);
	return 0;
}
