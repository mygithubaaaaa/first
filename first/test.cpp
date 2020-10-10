#include <iostream>
#include <fstream>
//#include <WinSock2.h>
#include <winsock2.h>
//#include <winsock.h>
#include <string.h>
#include <string>
using namespace std;

#pragma comment(lib,"ws2_32.lib")
#pragma warning(disable:4996)

const int DEFAULT_PORT = 8000;

#define HEADER "\
HTTP/1.1 200 OK\r\n\
Content-Type: text/html; charset=UTF-8\r\n\
Server: ZYhttp_v1.0.1\r\n\
Content-Length: %d\r\n\r\n%s\
"

#define HTML "\
<html>\
    <head>\
        <title>ZYhttp</title>\
    <head>\
    <body>\
        <h1>Hello ZYhttp!</h1>\
    </body>\
</html>\
"

char * gethtml(const char * filename) {
	if (filename == NULL)
	{
		cout << "filename error";
		return NULL;
	}
	ifstream ifile;
	char* buffer = new char[10240];
	ifile.open(filename,ios::in);
	if (!ifile) {
		cout << "open file error" << endl;
		return NULL;
	}
	ifile.getline(buffer, 10240, 0);
	//cout << buffer << endl;
	return buffer;
}

void getpeeraddr(SOCKET sockConn) {
	SOCKADDR_IN clientaddr;
	int len = sizeof(SOCKADDR);
	if (-1 == getpeername(sockConn, (SOCKADDR*)&clientaddr, &len)) {
		cout << "getpeername error" << endl;
	}
	cout << "ip:" << inet_ntoa((in_addr)clientaddr.sin_addr) << "\nport:" << clientaddr.sin_port << endl;
}

char* getrequest(char* buffer) {
	cout << "请求命令行:";
	char * request = new char[100];
	int i;
	for ( i =0;buffer[i]!='\r'&&i<100;i++)
	{
		cout << buffer[i];
		request[i] = buffer[i];
	}
	request[i] = '\0';
	cout << endl;
	return request;
}

int main() {
	WORD wVersionRequested;
	WSADATA wsaData;
	int err, ilen;
	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &wsaData);//初始化环境
	if (err != 0) {
		cout << "load win sock failed";
		return -1;
	}

	SOCKET sockSrv = socket(AF_INET, SOCK_STREAM, 0);
	if (sockSrv == INVALID_SOCKET) {
		cout << "socket() fail:" << WSAGetLastError() << endl;
		return -2;
	}

	//设置服务器地址
	SOCKADDR_IN addrSrv;
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_addr.s_addr = htonl(INADDR_ANY);
	addrSrv.sin_port = htons(DEFAULT_PORT);

	err = bind(sockSrv, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));
	if (err != 0) {
		cout << "bind() fail" << WSAGetLastError() << endl;
		return -3;
	}

	err = listen(sockSrv, 5);
	if (err != 0) {
		cout << "listen fail" << WSAGetLastError() << endl;
		return -4;
	}

	cout << "server waiting" << endl;

	SOCKADDR_IN addrclt;
	int len = sizeof(SOCKADDR);

	while (1)
	{
		SOCKET sockConn = accept(sockSrv, (SOCKADDR*)&addrclt, &len);
		char recvBuf[10240] = "\0";
		ilen = recv(sockConn, recvBuf, 10240, 0);
		recvBuf[ilen] = '\0';

		cout <<"服务器收到的浏览器请求\n"<< recvBuf << endl;

		getpeeraddr(sockConn);//获取并打印客户端（浏览器）的ip和port
		char* request = getrequest(recvBuf);//打印请求命令行

		char sendbuf[10240];
		char * html;
		string get = string(request);
		//cout << "string:get " << get << endl;
		if (get == "GET /index.html HTTP/1.1") {
			html = gethtml("index.html");
			if (html == NULL) {
				cout << "读取文件失败" << endl;
			}
		}
		else if(get =="GET /favicon.ico HTTP/1.1") {
			cout << "服务器回应:忽略";
			html = NULL;
		}
		else if (get == "GET /web.png HTTP/1.1") {
			html = gethtml("web.png");
			if (html == NULL) {
				cout << "读取文件失败" << endl;
			}
		}
		else {
			cout << "无法成功定位文件";
			html = NULL;
		}


		if (html != NULL) {
			sprintf(sendbuf, HEADER, strlen(html), html);
			cout << "服务器回应:返回报文";
			cout << "服务器发送(返回)的报文:\n" << sendbuf << endl;

			err = send(sockConn, sendbuf, strlen(sendbuf) + 1, 0);//给浏览器发送报文
			if (err < 0)
			{
				cout << "send data failed!" << endl;
				break;
			}
		}

		cout << endl;
		cout << endl;

		closesocket(sockConn);
		delete request;
		delete html;
	}
	closesocket(sockSrv);
	WSACleanup();

	return 0;
}

//int main() {
//	char * html = gethtml("D:\\作业\\lp473\\web\\index.html");
//	
//	return 0;
//
//}


