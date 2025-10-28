#include "server.hpp"

volatile sig_atomic_t g_running = 1;

void listeningLoop(Poller &poller)
{
	while (g_running)
	{
		poller.launchEpollListenLoop();
	}
}

void	checkConfigExist(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cout << "Usage: ./webserv configuration_file" << std::endl;
		exit(1);
	}

	std::ifstream configFile(argv[1]);
	if (!configFile.is_open())
	{
		std::cout << "The configuration file is not valid" << std::endl;
		exit(1);
	}
	configFile.close();
}

int main(int argc, char **argv)
{
	// SERVER CONFIGURATION

	// check the config file exists
	checkConfigExist(argc, argv);

	// parsing config file to create config objects with routes, ports, setup...
	std::list<ServerConf> serversConfs;
	serversConfs = ConfigParser::parseConfigFile(static_cast<const char *>(argv[1]));

	// constructing epoll instance to poll (i.e. watch) the server and client sockets
	Poller poller;

	// opening sockets listening on the configured ports, acting as servers
	std::vector<ServerSocket *> serverSockets;
	for (std::list<ServerConf>::iterator i = serversConfs.begin(); i != serversConfs.end(); i++)
		serverSockets.push_back(new ServerSocket(poller, *i));

	// initializing and handling signals
	signal(SIGINT, singalHandler);


	// SERVER RUNNING LOOP

	// using the poller to watch for any socket ready to write or read, and act accordingly
	listeningLoop(poller);


	// CLEAN UP

	// deleting the allocated info contained in the server sockets
	for (size_t i = 0; i < serverSockets.size(); i++)
		delete serverSockets[i];

	return 0;
}
