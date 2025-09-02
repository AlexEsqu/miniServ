#ifndef SERVER_H
# define SERVER_H

# include <stdio.h>
# include <sys/socket.h>
# include <unistd.h>
# include <stdlib.h>
# include <netinet/in.h>
# include <string>
# include <bits/stdc++.h>
#include <iostream>
#include <fstream> 
#include <string>
# define PORT 8080

enum e_methods
{
	GET,
	POST,
	DELETE,
	UNSUPPOTRTED
};
#endif
