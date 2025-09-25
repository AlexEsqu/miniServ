#include "server.hpp"


void listeningLoop(std::vector<ServerSocket*>& servers)
{
	while (1)
	{
		std::cout << "\n\n+++++++ Waiting for new request +++++++\n\n";

		servers[0]->launchEpollListenLoop();

	}
}

int main(int argc, char** argv)
{
	// reading config and setting up routes
	std::vector<ServerConf*>	serversConfs;
	if (argc > 1)
		serversConfs = ConfigParser::readConfigs(argv[1]);
	else
		serversConfs.push_back(new ServerConf());

	// constructing servers matching the configs
	std::vector<ServerSocket*>	servers;
	for (size_t i = 0; i < serversConfs.size(); i++)
		servers.push_back(new ServerSocket(*serversConfs[i]));

	// initializing and handling signals
	signal(SIGINT, singalHandler);

	listeningLoop(servers);

	return 0;
}

