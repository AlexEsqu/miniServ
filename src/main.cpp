#include "server.hpp"


void listeningLoop(std::vector<ServerSocket>& servers)
{
	ContentFetcher	cf;
	cf.addExecutor(new PHPExecutor());
	cf.addExecutor(new PythonExecutor());

	while (1)
	{
		std::cout << "\n\n+++++++ Waiting for new request +++++++\n\n";

		// create a socket to receive incoming communication
		ClientSocket AnsweringSocket(servers[0]);
		try {
			// reading the request into the Sockette buffer
			AnsweringSocket.readRequest();

			// decoding the buffer into a Request object
			Request decodedRequest(servers[0].getConf(), AnsweringSocket.getRequest());

			// creating a Response handling request according to configured routes
			Response response(servers[0].getConf(), decodedRequest);


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

int main(int argc, char** argv)
{
	// reading config and setting up routes
	std::vector<ServerConf>	serversConfs;
	if (argc > 1)
	{
		serversConfs = ConfigParser::readConfigs(argv[1]);
	}
	else
	{
		ServerConf defaultConf;
		serversConfs.push_back(defaultConf);
	}

	std::vector<ServerSocket>	servers;
	for (size_t i = 0; i < serversConfs.size(); i++)
	{
		ServerSocket	serv(serversConfs[i]);
		servers.push_back(serv);
	}

	listeningLoop(servers);

	return 0;
}

