#include "head.h"
#include "connection.h"
#include "server.h"

class Server {
	public:
		void init();
		void server_attr_init();
		void server_bind_sock();
		void server_listen_sock();
		int accept_conn();
		void close_conn();
		Server();
	private:
		sockaddr_in server;
		std::deque<Connection*> con_vec;
		Connection* cur;
};

void Server::init() {
	Connection * new_con = new Connection();
	con_vec.push_back(new_con);
	cur = con_vec.front();
	server_attr_init();
}

Server::Server() {
	this->init();
}


void Server::server_attr_init() {
	this->server.sin_family = AF_INET;
	this->server.sin_addr.s_addr = INADDR_ANY;
	this->server.sin_port = htons(80);
}

void Server::server_bind_sock() {
	if (bind(cur->get_sock(), (struct sockaddr *)&(this->server),sizeof(this->server)) < 0) {
		printf("Bind failed\n");
	}
}

void Server::server_listen_sock() {
	listen(cur->get_sock(), 3);
}

int Server::accept_conn() {
	int new_sock = accept(cur->get_sock(), NULL, NULL);
	if (new_sock == 0) return 0;
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
	char *char_buf = new char[3000];
	std::string buf;
	while ((new_sock = server->accept_conn())) {
		int rec_message = read(new_sock, char_buf, 2048);
		//std::cout << "Connected\n";
		//buf.assign(char_buf, char_buf + strlen(char_buf));
		std::regex split_re("\r\n");
		std::vector<std::string> request_message(std::sregex_token_iterator(buf.begin(), buf.end(), split_re, -1), std::sregex_token_iterator());
		//for (auto cur : request_message) std::cout << cur << '#' << std::endl;
		std::string message = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8 \r\n\r\n<h1>Hello!</h1>";
		write(new_sock, message.c_str(), message.size());
		server->close_conn();
	}
	return 0;
}
