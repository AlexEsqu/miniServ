#include "server.hpp"

void singalHandler(int sig)
{
	printf("Caught signal %d\n", sig);
	exit(sig);
}
