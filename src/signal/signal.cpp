#include "server.hpp"

void singalHandler(int sig)
{
	if (sig == SIGINT) {
		std::cout << "\nReceived SIGINT, shutting down gracefully..." << std::endl;
		g_running = 0;
	}
}
