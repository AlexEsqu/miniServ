#include "server.hpp"

Config conf;

void listeningLoop(Sockette &ListenerSocket)
{
	ContentFetcher	cf;
	cf.addExecutor(new PHPExecutor());

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

			// creating a Response handling request according to configured routes
			Response response(decodedRequest);


			// if CGI needed
			// 	response.setContent(execPHPwithFork(decodedRequest, requestedURL));
			cf.fillContent(response);

			write(AnsweringSocket.getSocketFd(), response.getHTTPResponse().c_str(), response.getHTTPResponse().size());

			std::cout << "\n\n+++++++ Answer has been sent +++++++ \n\n";
		}

		catch ( HTTPError &e )
		{
			std::cout << ERROR_FORMAT("\n\n+++++++ HTTP Error Page +++++++ \n\n");
			std::cout << "response is [" << e.getErrorPage() << "\n";
			write(AnsweringSocket.getSocketFd(), e.getErrorPage().c_str(), e.getErrorPage().size());
			std::cerr << e.what() << "\n";
		}

		catch ( std::exception &e )
		{
			std::cout << ERROR_FORMAT("\n\n+++++++ Non HTTP Error +++++++ \n\n");
			std::cerr << e.what() << "\n";
		}


	}
}

int main()
{
	// reading config and setting up routes
	// TO DO


	// initializing and handling signals
	signal(SIGINT, singalHandler);
	// creating a socket, binding it to an IP address and listening
	SocketteListen ListenerSocket(conf.getPort());

	listeningLoop(ListenerSocket);
	return 0;
}

