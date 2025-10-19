#pragma once
#include <iostream>
#include <sys/epoll.h>

#include "ClientSocket.hpp"
#include "ServerConf.hpp"
#include "ContentFetcher.hpp"
#include "PHPExecutor.hpp"
#include "PythonExecutor.hpp"
#include "Poller.hpp"

class ClientSocket;

class Poller;

class ContentFetcher;

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

	void				closeConnectionOrCleanAndKeepAlive(ClientSocket* client);
	void				receiveAndParseData(ClientSocket* client);
	void				sendDataIfReady(ClientSocket* client);

	bool				socketIsReadyToReceiveData(epoll_event& event);
	bool				socketIsReadyToSendData(epoll_event& event);
	bool				socketIsHavingTrouble(epoll_event& event);

};
