// Server side C program to demonstrate HTTP Server programming

#include "server.hpp"



int main()
{
	// response writer fct(status,method, content type, content length, url of the request)

	int server_fd, new_socket;
	long valread;
	struct sockaddr_in address;
	int addrlen = sizeof(address);

	std::string line;
	std::ifstream myHtmlFile("src/test/index.html");
	std::stringstream myHtmlText;
	myHtmlText << myHtmlFile.rdbuf();
	myHtmlFile.close();
	std::string hello = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: 319\n\n" + myHtmlText.str();
	std::cout << hello;
	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("In socket");
		exit(EXIT_FAILURE);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);
	memset(address.sin_zero, '\0', sizeof address.sin_zero);

	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		perror("In bind");
		exit(EXIT_FAILURE);
	}

	if (listen(server_fd, 10) < 0)
	{
		perror("In listen");
		exit(EXIT_FAILURE);
	}

	while (1)
	{
		printf("\n+++++++ Waiting for new connection ++++++++\n\n");
		if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
		{
			perror("In accept");
			exit(EXIT_FAILURE);
		}

		char buffer[30000] = {0};
		valread = read(new_socket, buffer, 30000);
		printf("%s\n", buffer);
		write(new_socket, hello.c_str(), hello.size());

		printf("------------------Hello message sent-------------------");
		close(new_socket);
	}

	return 0;
}
