#ifndef SERVER_H
# define SERVER_H

# include <stdio.h>
# include <sys/socket.h>
# include <sys/types.h>
# include <sys/wait.h>
# include <unistd.h>
# include <stdlib.h>
# include <netinet/in.h>
# include <string>
# include <bits/stdc++.h>
# include <iostream>
# include <string>
# include <map>
# include <csignal>
# include <exception>

# include "Status.hpp"
# include "Response.hpp"
# include "Request.hpp"
# include "Sockette.hpp"
# include "ClientSocket.hpp"
# include "ServerSocket.hpp"
# include "readability.hpp"
# include "ServerConf.hpp"
# include "Route.hpp"
# include "PHPExecutor.hpp"
# include "PythonExecutor.hpp"
# include "ContentFetcher.hpp"
# include "ConfigParser.hpp"


enum e_CGI
{
	NO_CGI,
	PY,
	PHP
};


extern volatile sig_atomic_t g_running;
void singalHandler(int sig);


#endif
