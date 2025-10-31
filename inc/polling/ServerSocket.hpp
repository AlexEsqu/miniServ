#pragma once
#include <iostream>
#include <sys/epoll.h>
#include <list>

#include "ClientSocket.hpp"
#include "ServerConf.hpp"
#include "ContentFetcher.hpp"
#include "PHPExecutor.hpp"
#include "PythonExecutor.hpp"
#include "Poller.hpp"
#include "Session.hpp"

static const time_t	TIMEOUT_CONNECTION = 10;

class ClientSocket;

class Poller;

class ContentFetcher;

class ServerSocket: public Sockette
{

private:

	Poller&							_poller;
	ServerConf&						_conf;
	ContentFetcher*					_cf;
	std::map<int, ClientSocket*>	_clients;
	std::map<size_t, Session> 		_sessionMap;


public:

	//----------------- CONSTRUCTORS ---------------------//

	ServerSocket(Poller& poller, ServerConf& conf);

	//----------------- DESTRUCTOR -----------------------//

	~ServerSocket();

	//------------------- OPERATORS ----------------------//


	//--------------------- GETTER -----------------------//

	ServerConf&			getConf();
	Poller&				getEpoll();
	std::map<size_t, Session>&	getSessionMap();

	//--------------- MEMBER FUNCTIONS -------------------//

	void				handleConnection(epoll_event event);

	void				acceptNewConnection();
	void				removeConnection(ClientSocket* clientSocket);
	void				handleExistingConnection(ClientSocket* client, epoll_event &event);

	void				closeConnectionOrCleanAndKeepAlive(ClientSocket* client);
	void				timeoutIdleClients();
	void				timeoutRequest(ClientSocket& client);

	void				receiveAndParseData(ClientSocket* client);
	void				sendDataIfComplete(ClientSocket* client);

	bool				socketIsReadyToReceiveData(epoll_event& event);
	bool				socketIsReadyToSendData(epoll_event& event);
	bool				socketIsHavingTrouble(epoll_event& event);
	bool				socketIsHangingUp(epoll_event& event);


};
