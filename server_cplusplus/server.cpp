#include "head.h"
#include "connection.h"
#include "server.h"


void Server::init() {
	Connection * new_con = new Connection();
	con_vec.push_back(new_con);
	cur = con_vec.front();
	server_attr_init();
}

Server::Server() {
	this->init();
}

Connection* Server:: get_cur_conn() {
	return cur;
}

void Server::server_attr_init() {
	this->server.sin_family = AF_INET;
	this->server.sin_addr.s_addr = INADDR_ANY;
	this->server.sin_port = htons(80);
}

void Server::server_bind_sock() {
	if (bind(cur->get_sock(), (struct sockaddr *)&(this->server),sizeof(this->server)) < 0) {
		handle_error("bind");
	}
}

void Server::server_listen_sock() {
	listen(cur->get_sock(), 3);
}

int Server::accept_conn() {
	int new_sock = accept(cur->get_sock(), NULL, NULL);
	Connection *new_conn = new Connection(new_sock);
	cur = new_conn;
	con_vec.push_front(cur);
	return new_sock;
}

void Server::close_conn() {
	cur->close_connection();
	con_vec.pop_front();
	cur = con_vec.front();
}

int main() {
	Server *server = new Server();
	server->server_bind_sock();
	server->server_listen_sock();
	int new_sock;
	while ((new_sock = server->accept_conn())) {
		Connection* cur = server->get_cur_conn();
		cur->http_handler();
		server->close_conn();
	}
	return 0;
}
