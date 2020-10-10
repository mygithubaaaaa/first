#include <iostream>
#include <fstream>
#include <winsock2.h>
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

