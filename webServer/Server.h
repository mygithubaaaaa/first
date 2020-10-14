#pragma once
#include <winsock2.h>
#include <list>
#include <string>
#include <vector>


class Server
{
public:
	Server();
	~Server();

	int WinsockStartup();
	int ServerStartup();
	int ListenStartup();
	void getpeeraddr(SOCKET sockConn);
	int loop();
	int response2(SOCKET);
	#define buf_length 102400


private:
	SOCKET srvSocket;
	sockaddr_in srvAddr;
	char * recvBuf;
	std::vector<SOCKET> sessions;
	std::string maincontents;

};
