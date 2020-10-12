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

bool endWith(std::string url, std::string type) {
	if (url.size() < type.size()) {
		return false;
	}
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
	for (int i=0;i<this->sessions.size();i++)
	{
		if (this->sessions[i] != NULL) {
			closesocket(this->sessions[i]);
			this->sessions[i] = NULL;
		}
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
	//std::cout << "请输入监听端口" << std::endl;

	std::cout << "Server socket create ok!" << std::endl;
	
	//设置服务器IP地址和端口号
	this->srvAddr.sin_family = AF_INET;

	ifstream readconfig("config.txt");
	if (!readconfig)
	{
		cout << "配置文件读取失败，检查目录下是否存在config.txt" << endl;
		return 0;
	}
	readconfig.seekg(0, std::ios::beg);
	readconfig.seekg(0, std::ios::end);
	int configlength = readconfig.tellg();//获取配置文件大小
	readconfig.seekg(0, std::ios::beg);
	char* temp = new char[configlength];
	readconfig.getline(temp, configlength, 0);
	readconfig.close();//关闭文件
	string s = string(temp);
	

	std::regex ip(R"(([0-9]+).([0-9]+).([0-9]+).([0-9]+))");//匹配ip
	std::regex port(R"(port:([0-9]+))");//匹配port号
	std::smatch matchip, matchport;
	std::string Serverip, Serverport;
	if (std::regex_search(s, matchip, ip)) {
		Serverip = matchip[0].str();
	}
	if (std::regex_search(s, matchport, port)) {
		Serverport = matchport[1].str();
	}
	cout << "读取的配置:\n" << "ip:" << Serverip << "\nport:" << Serverport << endl;

	this->srvAddr.sin_port = htons(stoi(Serverport));
	//this->srvAddr.sin_addr.s_addr = inet_addr(Serverip.c_str());
	this->srvAddr.sin_addr.s_addr = INADDR_ANY;
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
	int rtn = listen(this->srvSocket, 10);//最大连接数10
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

void Server::stopServer() {

}


int Server::loop() {
	int threadnum = 0;
	while (1)
	{
		SOCKADDR_IN addrclt;
		int len = sizeof(SOCKADDR);

		SOCKET sock= accept(this->srvSocket, (SOCKADDR*)&addrclt, &len);
		this->sessions.push_back(sock);
		thread t(&Server::response2,this,this->sessions[threadnum]);
		t.detach();
		threadnum++;
	}
}


int Server::response2(SOCKET sock) {
	char recvBuf[buf_length];
	char sendBuf[buf_length];

	std::string firstHeader, typeStr, lengthStr;
	firstHeader = "HTTP/1.1 200 OK\r\n";
	memset(recvBuf, 0, sizeof(recvBuf));
	int rtn = 1;
	while (rtn !=0 && rtn != SOCKET_ERROR) {
		rtn = recv(sock, recvBuf, sizeof(recvBuf), 0);
		if (rtn == SOCKET_ERROR || rtn == 0) {
			//break;
			return 0;//接收失败或对方断开连接
		}
		std::string strRecv, recvHeader;
		strRecv.assign(recvBuf);
		auto headerEnd = strRecv.find("\r\n\r\n");
		recvHeader = strRecv.substr(0, headerEnd);
		ostringstream output;

		//cout << "服务器收到的http报文头:" << recvHeader << std::endl;
		//output << "服务器收到的http报文头:" << recvHeader << std::endl;
		std::regex regRequest(R"(([A-Z]+) (.*?) HTTP/\d.\d)");
		std::smatch matchRequest;
		std::string method, url;
		if (std::regex_search(recvHeader, matchRequest, regRequest)) {
			method = matchRequest[1].str();//匹配到请求类型
			url = matchRequest[2].str();
		}
		output << "=========================\n浏览器IP和port:" << std::endl;

		//输出浏览器ip和port
		SOCKADDR_IN clientaddr;
		int len = sizeof(SOCKADDR);
		if (-1 == getpeername(sock, (SOCKADDR*)&clientaddr, &len)) {
			std::cout << "getpeername error" << std::endl;
			return 0;
		}
		output << "ip:" << inet_ntoa((in_addr)clientaddr.sin_addr) << "\nport:" << clientaddr.sin_port << std::endl;


		//getpeeraddr(sock);
		output<< "请求命令行: " << matchRequest[0].str() <<endl;
		//std::cout << "method:" << method << std::endl;
		if (endWith(url, ".html") == true||endWith(url,"htm")== true) {
			typeStr = "Content-Type: text/html\r\n";
		}
		else if (endWith(url, ".jpg") == true) {
			typeStr = "Content-Type: image/jpeg\r\n";
		}
		else if (endWith(url, ".ico") == true ){
			typeStr = "Content-Type: image/x-icon\r\n";
		}
		else if (endWith(url, ".png") == true) {
			typeStr = "Content-Type: image/png\r\n";
		}
		else if (endWith(url, ".gif") == true) {
			typeStr = "Content-Type: image/gif\r\n";
		}
		else if (endWith(url, ".css") == true) {
			typeStr = "Content-Type: text/css\r\n";
		}
		else if (endWith(url, ".js") == true) {
			typeStr = "Content-Type: application/x-javascript\r\n";
		}
		else if (endWith(url, ".ttf") == true) {
			typeStr = "Content-Type: application/octet-stream\r\n";
		}
		else if (endWith(url, ".woff") == true) {
			typeStr = "Content-Type: application/x-font-woff\r\n";
		}
		//else 
		//{
		//	//501

		//	firstHeader = "HTTP/1.1 501 Not Inplemented\r\n";
		//	typeStr = "Content-Type: text/html\r\n";
		//	output << "file name:" << url << " file open error,发送404" << std::endl;
		//	std::string httpHeader = firstHeader + typeStr + "\r\n";
		//	send(sock, httpHeader.c_str(), httpHeader.length(), 0);
		//	cout << output.str();
		//	continue;
		//}
		if (url[0] == '/') {
			url = url.substr(1, url.size());//去掉/
		}
		FILE *infile = fopen(url.c_str(), "rb");
		if (infile == NULL) {
			//output << typeStr << std::endl;
			output << "file name:" << url << " file open error,发送404" << std::endl;

			//std::cout << output.str();
			//fclose(infile);
			//return 0;
			url = "404.html";
			infile = fopen(url.c_str(), "rb");
			
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
		//std::cout << "response header:\n" << responseHeader << std::endl;
		send(sock, responseHeader.c_str(), (responseHeader.length()), 0);
		//std::cout << "will send " << std::endl;
		while (true)
		{
			memset(sendBuf, 0, sizeof(sendBuf));
			int bufReadNum = fread(sendBuf, 1, buf_length, infile);
			//cout << "send buf" << sendBuf << endl;
			if (SOCKET_ERROR == send(sock, sendBuf, bufReadNum, 0)) {
				output << "send error,will break \n=========================" << std::endl;
				break;
			}
			if (feof(infile)) {
				output << "发送"<<url<<"完成\n=========================" << std::endl;
				break;
			}
			//std::cout << "sending" << std::endl;
		}
		cout << output.str();
		fclose(infile);
	}
	closesocket(sock);
	sock = NULL;
	return 1;
}