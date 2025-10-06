#include "server.hpp"

volatile sig_atomic_t g_running = 1;

void listeningLoop(Poller& poller)
{
	while (g_running)
	{
		poller.launchEpollListenLoop();
	}
}

int main(int , char** argv)
{
	// parsing config and setting up routes, or if no config setting up default
	std::vector<ServerConf>	serversConfs;
	serversConfs = ConfigParser::parseConfigFile(static_cast<const char*>(argv[1]));

	// constructing servers matching the configs
	Poller	poller;

	std::vector<ServerSocket*>	servers;
	for (size_t i = 0; i < serversConfs.size(); i++)
		servers.push_back(new ServerSocket(poller, serversConfs[i]));

	// initializing and handling signals
	signal(SIGINT, singalHandler);

	listeningLoop(poller);

	for (size_t i = 0; i < servers.size(); i++)
		delete servers[i];

	return 0;
}

