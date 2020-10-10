#pragma once

#include "Server.h"
#include "WinsockEnv.h"
#include <string>
#include <winsock2.h>
#include <iostream>
#include <fstream>
#include <regex>
#include <sstream>
#include <thread>
using namespace std;


#pragma comment(lib,"ws2_32.lib")
#pragma warning(disable:4996)
const int PORT = 8000;

bool endWith(std::string url, std::string type) {
	if (url.compare(url.size() - type.size(), type.size(),type) == 0) {
		return true;
	}
	return false;
}

Server::Server() {

}
Server::~Server() {
	//关闭server socket
	if (this->srvSocket != NULL) {
		closesocket(this->srvSocket);
		this->srvSocket = NULL;
	}
}


int Server::WinsockStartup() {
	if (WinsockEnv::Startup() == -1) return -1;
	return 0;
}

int Server::ServerStartup() {
	this->srvSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (this->srvSocket == INVALID_SOCKET) {
		std::cout << "Server socket create error!" << std::endl;
		WSACleanup();
		return -1;
	}
	std::cout << "请输入监听端口" << std::endl;

	std::cout << "Server socket create ok!" << std::endl;
	
	//设置服务器IP地址和端口号
	this->srvAddr.sin_family = AF_INET;
	this->srvAddr.sin_port = htons(PORT);
	this->srvAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	//绑定 socket to Server's IP and port
	int rtn = ::bind(this->srvSocket, (LPSOCKADDR)&(this->srvAddr), sizeof(this->srvAddr));
	if (rtn == SOCKET_ERROR) {
		std::cout << "Server socket bind error!\n";
		closesocket(this->srvSocket);
		WSACleanup();
		return -1;
	}

	std::cout << "Server socket bind ok!\n";
	return 0;

}

int Server::ListenStartup() {
	int rtn = listen(this->srvSocket, 10);//最大连接数5
	if (rtn == SOCKET_ERROR) {
		std::cout << "Server socket listen error!\n";
		closesocket(this->srvSocket);
		WSACleanup();
		return -1;
	}

	std::cout << "Server socket listen ok!\n";
	return 0;
}

void Server::getpeeraddr(SOCKET sockConn) {
	SOCKADDR_IN clientaddr;
	int len = sizeof(SOCKADDR);
	if (-1 == getpeername(sockConn, (SOCKADDR*)&clientaddr, &len)) {
		std::cout << "getpeername error" << std::endl;
		return;
	}
	std::cout << "ip:" << inet_ntoa((in_addr)clientaddr.sin_addr) << "\nport:" << clientaddr.sin_port << std::endl;
}

char* Server::getrequest(char* buffer) {
	std::cout << "请求命令行:" << buffer << std::endl;
	char * request = new char[100];
	int i;
	for (i = 0; buffer[i] != '\r'&&i < 100; i++)
	{
		std::cout << buffer[i];
		request[i] = buffer[i];
	}
	request[i] = '\0';
	std::cout << std::endl;
	return request;
}

int Server::loop() {
	//SOCKADDR_IN addrclt;
	int len = sizeof(SOCKADDR);
	int threadnum = 0;
	//this->sessions[threadnum] = accept(this->srvSocket, (SOCKADDR*)&addrclt, &len);
	while (1)
	{
		SOCKADDR_IN addrclt;
		SOCKET sock= accept(this->srvSocket, (SOCKADDR*)&addrclt, &len);
		this->sessions.push_back(sock);
		thread t(&Server::response2,this,((this->sessions[threadnum])));
		t.detach();
		cout << threadnum << endl;
		threadnum++;
		//char recvBuf[10240] = "\0";
		//ilen = recv(this->sessions, recvBuf, 10240, 0);
		//if (ilen <= 0) {
		//	std::cout << "=============" << endl;
		//	continue;
		//}
		//recvBuf[ilen] = '\0';

		//cout << "服务器收到的浏览器请求\n" << recvBuf << endl;

		//getpeeraddr(this->sessions);//获取并打印客户端（浏览器）的ip和port
		//char* request = getrequest(recvBuf);//打印请求命令行
		//this->response(request);//发送回应报文
		//closesocket(this->sessions);
	}
}

//void Server::response(char * request) {
//	char sendbuf[102400];
//	char * html;
//	int err;
//	string get = string(request);
//	cout << "string:get " << get << endl;
//	if (get == "GET /index.html HTTP/1.1") {
//		html = this->gethtml("index.html");
//		if (html == NULL) {
//			cout << "读取文件失败" << endl;
//		}
//	}
//	else if (get == "GET /favicon.ico HTTP/1.1") {
//		cout << "服务器回应:忽略";
//		html = NULL;
//	}
//	else if (get == "GET /web.png HTTP/1.1") {
//		html = this->gethtml("web.png");
//		if (html == NULL) {
//			cout << "读取文件失败" << endl;
//		}
//	}
//	else if (get == "GET /hust.jpg HTTP/1.1") {
//		html = this->getpicture("hust.jpg");
//		if (html == NULL) {
//			cout << "文件读取失败" << endl;
//		}
//	}
//	else if(get =="GET /jiwang.html HTTP/1.1"){
//		html = this->gethtml("D:\\作业\\计算机网络\\实验1socket编程\\first\\webServer\\jiwang.html");
//		if (html == NULL) {
//			cout << "文件读取失败" << endl;
//		}
//
//	}
//	else {
//		cout << "无法成功定位文件";
//		html = NULL;
//	}
//
//	if (html != NULL) {
//		if (get == "GET /hust.jpg HTTP/1.1") {
//			int size = strlen(html);
//			char header[] = "HTTP/1.1 200 OK\r\nContent-Type: image/jpeg\r\naccept-ranges:bytes\r\nContent-Length:31427\r\n\r\n";
//			memcpy(sendbuf, header, strlen(header));
//			this->headersize = strlen(header);
//			memcpy(sendbuf + strlen(header), html, this->picturelength);
//		}
//		else {
//			sprintf(sendbuf, HEADER, strlen(html), html);
//			this->headersize = strlen(sendbuf);
//
//		}
//		cout << "服务器回应:返回报文" << endl;;
//		cout << "服务器发送(返回)的报文:\n" << sendbuf << endl;
//
//		err = send(this->sessions, sendbuf, this->headersize+this->picturelength, 0);//给浏览器发送报文
//		this->picturelength = 0;
//		if (err < 0)
//		{
//			cout << "send data failed!" << endl;
//		}
//
//		//this->sendpicture(this->sessions);
//	}
//}

int Server::response2(SOCKET sock) {
	char recvBuf[buf_length];
	char sendBuf[buf_length];

	std::string firstHeader, typeStr, lengthStr;
	firstHeader = "HTTP/1.1 200 OK\r\n";
	memset(recvBuf, 0, sizeof(recvBuf));
	int rtn = 1;
	while (rtn!=0&&rtn!=SOCKET_ERROR) {
		rtn = recv(sock, recvBuf, sizeof(recvBuf), 0);
		//if (rtn == SOCKET_ERROR || rtn == 0) {
		//	//break;
		//	return 0;//接收失败或对方断开连接
		//}
		std::string strRecv, recvHeader;
		strRecv.assign(recvBuf);
		auto headerEnd = strRecv.find("\r\n\r\n");
		recvHeader = strRecv.substr(0, headerEnd);
		std::cout << "浏览器IP和port:" << std::endl;
		getpeeraddr(sock);
		std::cout << "服务器收到的http报文头:" << recvHeader << std::endl;
		std::regex regRequest(R"(([A-Z]+) (.*?) HTTP/\d.\d)");
		std::smatch matchRequest;
		std::string method, url;
		if (std::regex_search(recvHeader, matchRequest, regRequest)) {
			method = matchRequest[1].str();//匹配到请求类型
			url = matchRequest[2].str();
		}
		std::cout << "method:" << method << std::endl;

		if (endWith(url, ".jpg") == 0) {
			typeStr = "Content-Type: text/html\r\n";
		}
		else if (endWith(url, ".jpg") == 0) {
			typeStr = "Content-Type: image/jpeg\r\n";
		}
		else if (endWith(url, ".ico") == 0) {
			typeStr = "Content-Type: image/x-icon\r\n";
		}
		else if (endWith(url, ".png") == 0) {
			typeStr = "Content-Type: image/png\r\n";
		}
		else if (endWith(url, ".gif") == 0) {
			typeStr = "Content-Type: image/gif\r\n";
		}
		else {
			//501

			firstHeader = "HTTP/1.1 501 Not Inplemented\r\n";
			typeStr = "Content-Type: text/html\r\n";
			url = "501.html";

		}
		if (url[0] == '/') {
			url = url.substr(1, url.size());//去掉
		}
		std::cout << url << std::endl;
		FILE *infile = fopen(url.c_str(), "rb");
		if (infile == NULL) {
			std::cout << typeStr << std::endl;
			std::cout << "file name:" << url << "file open error" << std::endl;
			fclose(infile);
			return 0;
		}
		std::fseek(infile, 0, SEEK_SET);
		std::fseek(infile, 0, SEEK_END);
		int filelength = ftell(infile);
		std::fseek(infile, 0, SEEK_SET);
		std::string responseHeader = firstHeader
			+ typeStr
			+ "Content-Length: " + std::to_string(filelength) + "\r\n"
			//+ "Connection: close\r\n"
			+ "\r\n";
		std::cout << "response header:\n" << responseHeader << std::endl;
		send(sock, responseHeader.c_str(), (responseHeader.length()), 0);
		std::cout << "will send " << std::endl;
		while (true)
		{
			memset(sendBuf, 0, sizeof(sendBuf));
			int bufReadNum = fread(sendBuf, 1, buf_length, infile);
			//cout << "send buf" << sendBuf << endl;
			if (SOCKET_ERROR == send(sock, sendBuf, bufReadNum, 0)) {
				std::cout << "send error,will break " << std::endl;
				break;
			}
			if (feof(infile)) {
				std::cout << " send file complete,will break " << std::endl;
				break;
			}
			std::cout << "sending" << std::endl;
		}
		fclose(infile);
	}
	closesocket(sock);
	return 1;
}

char * Server::gethtml(const char * filename) {
	if (filename == NULL)
	{
		std::cout << "filename error";
		return NULL;
	}
	std::ifstream ifile;
	char* buffer = new char[10240];
	ifile.open(filename, std::ios::in);
	if (!ifile) {
		std::cout << "open file error" << std::endl;
		return NULL;
	}
	ifile.getline(buffer, 10240, 0);
	//cout << buffer << endl;
	return buffer;
}


char * Server::getpicture(const char *picturename) {
	if (picturename == NULL)
	{
		std::cout << "picture name error";
		return NULL;
	}
	int err;
	std::ifstream is(picturename, std::ifstream::in | std::ios::binary);
	if (!is) {
		std::cout << "open file error " << std::endl;
		return NULL;
	}
	is.seekg(0, is.end);
	int length = is.tellg();
	std::cout << "图片长度" << length << std::endl;
	is.seekg(0, is.beg);
	// 3. 创建内存缓存区
	char * buffer = new char[length];
	// 4. 读取图片
	is.read(buffer, length);
	return buffer;
}

void Server::sendpicture(SOCKET sock) {
	//char buffer[10240];
	//memset(buffer, 0, sizeof(buffer));
	//FILE *p;
	//p = fopen("C:\\Users\\liupan\\Pictures\\截图\\aaa.jpg","r");
	//if (p == NULL) {
	//	cout << "picture file open error" << endl;
	//	return;
	//}
	//
	int err;
	std::ifstream is("D:\\作业\\计算机网络\\实验1socket编程\\first\\webServer\\hust.jpg", std::ifstream::in | std::ios::binary);
	is.seekg(0, is.end);
	int length = is.tellg();
	is.seekg(0, is.beg);
	// 3. 创建内存缓存区
	char * buffer = new char[length];
	// 4. 读取图片
	is.read(buffer, length);
	char *buffer2 = new char[length + 1024];
	sprintf(buffer2, HEADER, strlen(buffer), buffer);
	err = send(sock, buffer2, length+1024, 0);
	std::cout << "picture length:" << length << "strlen(buffer2):" << strlen(buffer2) << std::endl;
	std::cout << "header:"<<HEADER << std::endl;
	std::cout << "服务器发送(返回)的图片报文:\n" << buffer2 << std::endl;
	if (err < 0)
	{
		std::cout << "send picture failed!" << std::endl;
	}

}



