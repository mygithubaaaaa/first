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
	void response(char *);
	char* getrequest(char* buffer);
	int loop();
	char * gethtml(const char * filename);
	void sendpicture(SOCKET sock);
	char * getpicture(const char *picturename);
	int response2(SOCKET);

#define HEADER "\
HTTP/1.1 200 OK\r\n\
Content-Type: text/html; charset=UTF-8\r\n\
Server: ZYhttp_v1.0.1\r\n\
Content-Length: %d\r\n\r\n%s\
"
	#define buf_length 102400


private:
	SOCKET srvSocket;
	sockaddr_in srvAddr;
	char * recvBuf;
	std::vector<SOCKET> sessions;
	int picturelength = 0;
	int headersize;

};
