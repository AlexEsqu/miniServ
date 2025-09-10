#include "server.hpp"

void listeningLoop(Sockette &ListenerSocket)
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

		std::string	testPHPFile = "./pages/action.php";
		execPHPwithFork(testPHPFile);


		// creating a Response
		Response response(200, "text/html", decodedRequest.getRequestedURL());
		write(AnsweringSocket.getSocketFd(), response.getHTTPResponse().c_str(), response.getHTTPResponse().size());

		std::cout << "\n\n+++++++ Answer has been sent +++++++ \n\n" << std::endl;
	}
}

int main()
{
	// creating a socket, binding it to an IP address and listening
	SocketteListen ListenerSocket(PORT);

	listeningLoop(ListenerSocket);

	return 0;
}
