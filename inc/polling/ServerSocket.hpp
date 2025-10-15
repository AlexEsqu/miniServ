#pragma once
#include <iostream>
#include <sys/epoll.h>

#include "ClientSocket.hpp"
#include "ServerConf.hpp"
#include "ContentFetcher.hpp"
#include "HTTPError.hpp"
#include "PHPExecutor.hpp"
#include "PythonExecutor.hpp"
#include "Poller.hpp"

enum e_clientSocketMode
{
	WRITING,
	READING
};


class ClientSocket;

class Poller;

class ServerSocket: public Sockette
{

private:

	Poller&							_poller;
	const ServerConf&				_conf;
	ContentFetcher*					_cf;
	std::map<int, ClientSocket*>	_clients;

public:

	//----------------- CONSTRUCTORS ---------------------//

	ServerSocket(Poller& poller, const ServerConf& conf);

	//----------------- DESTRUCTOR -----------------------//

	~ServerSocket();

	//------------------- OPERATORS ----------------------//


	//--------------------- GETTER -----------------------//

	const ServerConf&	getConf() const;
	Poller&				getEpoll();

	//--------------- MEMBER FUNCTIONS -------------------//

	void				acceptNewConnection();
	void				removeConnection(ClientSocket* clientSocket);
	void				handleExistingConnection(ClientSocket* client, epoll_event &event);

	void				setPollingMode(e_clientSocketMode mode, ClientSocket* client);
	void				closeConnectionOrResetAndKeepAlive(ClientSocket* client);

};
