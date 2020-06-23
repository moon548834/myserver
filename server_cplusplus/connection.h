#include "head.h"
#include <cerrno>
#include <boost/filesystem.hpp>

class Server {
	public:
		void init();
		void server_attr_init();
		void server_bind_sock();
		void server_listen_sock();
		Connection* get_cur_conn();
		int accept_conn();
		void close_conn();
		Server();
	private:
		sockaddr_in server;
		std::deque<Connection*> con_vec;
		Connection* cur;
};

class Connection {
	public:
		int create_connection();
		void close_connection();
		void http_handler();
		int get_sock();
		Connection();
		Connection(int sock);
		std::string get_http_response(std::string filename);
		std::string get_file_content(std:: string firedir);
	private:
		int sock;
};

const std::string http_message_head_200= 
	"HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\n\r\n";

const std::string http_message_head_404= 
	"HTTP/1.1 404 Not Found\r\nContent-Type: text/html; charset=utf-8\r\n\r\n";

const std::string root_dir = "/root/work/myserver/front_end/";

const std::string default_filename = "index.html";
const std::string not_found_filename = "404.html";
