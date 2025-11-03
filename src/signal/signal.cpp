#include "server.hpp"

void singalHandler(int sig)
{
	if (sig == SIGINT) {
		std::cout << "\nReceived SIGINT, shutting down..." << std::endl;
		g_running = 0;
	}
}
