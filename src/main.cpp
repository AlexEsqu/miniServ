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

void	listeningLoop(Sockette &ListenerSocket)
{
	while (1)
	{
		std::cout << "\n\n+++++++ Waiting for new request +++++++\n\n" << std::endl;

		// create a socket to receive incoming communication
		SocketteAnswer AnsweringSocket(ListenerSocket);

		// reading the request into the Sockette buffer
		AnsweringSocket.readRequest();

		// decoding the buffer into a Request object
		Request decodedRequest(AnsweringSocket.getRequest());

		// creating a Response
		std::string finalResponse = createResponse("pages/index.html");
		write(AnsweringSocket.getSocketFd(), finalResponse.c_str(), finalResponse.size());

		std::cout << "\n\n+++++++ Answer has been sent +++++++ \n\n" << std::endl;
	}
}


int main()
{
	// creating a socket, binding it to an IP address and listening
	SocketteListen	ListenerSocket(PORT);

	listeningLoop(ListenerSocket);

	return 0;
}
