#include "server.hpp"

void listeningLoop(std::vector<ServerSocket*>& servers)
{
	while (1)
	{
		for (size_t i = 0; i < servers.size(); i++)
			servers[i]->launchEpollListenLoop();
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

