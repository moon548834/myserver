#include "connection.h"
#include "head.h"
#include <boost/filesystem.hpp>
Connection::Connection() {
	create_connection();
}

Connection::Connection(int sock) {
	this->sock = sock;
}

int Connection::create_connection() {
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		printf("Can not create a new socket\n");
	}
	return sock;
}

void Connection::close_connection() {
	close(sock);
}

int Connection::get_sock() {
	return sock;
}


