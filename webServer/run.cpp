#include "server.h"

int main() {
	Server server;
	if (server.WinsockStartup() == -1) return 0;
	if (server.ServerStartup() == -1) return 0;
	if (server.ListenStartup() == -1) return 0;
	if (server.loop() == -1) return 0;

}