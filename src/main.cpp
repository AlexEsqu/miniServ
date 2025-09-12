#include "server.hpp"

void listeningLoop(Sockette &ListenerSocket)
{
	while (1)
	{
		std::cout << "\n\n+++++++ Waiting for new request +++++++\n\n";

		try {
			// create a socket to receive incoming communication
			SocketteAnswer AnsweringSocket(ListenerSocket);

			// reading the request into the Sockette buffer
			AnsweringSocket.readRequest();

			// decoding the buffer into a Request object
			Request decodedRequest(AnsweringSocket.getRequest());
			std::string	requestedURL = decodedRequest.getRequestedURL();

			// creating a Response
			decodedRequest.testFilename();

			decodedRequest.redirectIfCGI();
			Response response(200, decodedRequest.getContentType(), decodedRequest.getRequestedURL());
			// response.setContent(execPHPwithFork(decodedRequest, requestedURL));
			write(AnsweringSocket.getSocketFd(), response.getHTTPResponse().c_str(), response.getHTTPResponse().size());

			std::cout << "\n\n+++++++ Answer has been sent +++++++ \n\n";
		}

		catch ( std::exception &e )
		{
			std::cout << ERROR_FORMAT("\n\n+++++++ Answer has not been been sent +++++++ \n\n");
			std::cerr << e.what() << "\n";
		}


	}
}

int main()
{
	// initializing and handling signals
	signal(SIGINT, singalHandler);

	// creating a socket, binding it to an IP address and listening
	SocketteListen ListenerSocket(PORT);

	listeningLoop(ListenerSocket);

	return 0;
}

