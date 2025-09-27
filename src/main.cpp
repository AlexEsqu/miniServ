#include "server.hpp"

void listeningLoop(std::vector<ServerSocket*>& servers)
{
	while (1)
	{
		std::cout << CGI_FORMAT("\n+++++++ Waiting for new request +++++++\n");

		servers[0]->launchEpollListenLoop();

	}
}

int main(int , char** argv)
{
	// parsing config and setting up routes, or if no config setting up default
	std::vector<ServerConf*>	serversConfs;
	serversConfs = ConfigParser::parseConfigFile(argv[1]);

	// constructing servers matching the configs
	std::vector<ServerSocket*>	servers;
	for (size_t i = 0; i < serversConfs.size(); i++)
		servers.push_back(new ServerSocket(*serversConfs[i]));

	// initializing and handling signals
	signal(SIGINT, singalHandler);

	listeningLoop(servers);

	return 0;
}

