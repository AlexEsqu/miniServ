#include "server.hpp"

void listeningLoop(Sockette &ListenerSocket)
{
	while (1)
	{
		std::cout << "\n\n+++++++ Waiting for new request +++++++\n\n";

		// create a socket to receive incoming communication
		SocketteAnswer AnsweringSocket(ListenerSocket);

		try {
			// reading the request into the Sockette buffer
			AnsweringSocket.readRequest();

			// decoding the buffer into a Request object
			Request decodedRequest(AnsweringSocket.getRequest());
			std::string	requestedURL = decodedRequest.getRequestedURL();

			// creating a Response
			decodedRequest.testFilename();

			decodedRequest.redirectIfCGI();
			Response response(decodedRequest, 200);
			// response.setContent(execPHPwithFork(decodedRequest, requestedURL));
			write(AnsweringSocket.getSocketFd(), response.getHTTPResponse().c_str(), response.getHTTPResponse().size());

			std::cout << "\n\n+++++++ Answer has been sent +++++++ \n\n";
		}

		catch ( Request::HTTPError &e )
		{
			std::cout << ERROR_FORMAT("\n\n+++++++ Been Been +++++++ \n\n");
			std::cout << "response is [" << e.getErrorPage() << "\n";
			write(AnsweringSocket.getSocketFd(), e.getErrorPage().c_str(), e.getErrorPage().size());
			std::cerr << e.what() << "\n";
		}

		catch ( std::exception &e )
		{
			std::cout << ERROR_FORMAT("\n\n+++++++ Not Not Been Been +++++++ \n\n");
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

