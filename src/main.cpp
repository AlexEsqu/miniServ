#include "server.hpp"

int main(int argc, char**argv)
{
	//SERVER CONFIGURATION

	// check the config file exists, is readable, provides at least one good configuration...
	std::vector<ServerConf> serversConfs;
	try
	{
		serversConfs = ConfigParser::parseConfig(argc, argv);
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}

	// constructing epoll instance to poll (i.e. watch) the server and client sockets
	Poller poller;

	// opening sockets listening on the configured ports, acting as servers
	poller.openServersAndAddToWatchList(serversConfs);

	// initializing and handling signals
	signal(SIGINT, singalHandler);


	// SERVER RUNNING LOOP

	// using the poller to watch for any socket ready to write or read, and act accordingly
	poller.launchEpollListenLoop();


	return 0;
}
