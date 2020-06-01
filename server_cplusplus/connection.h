#include "head.h"
#include <cerrno>
#include <boost/filesystem.hpp> 

class Connection {
	public:
		int create_connection();
		void close_connection();
		void handle_req();
		int get_sock();
		Connection();
		Connection(int sock);
		std::string get_response(std::string filename);
	private:
		int sock;
};

const std::string response_head200 = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\n\r\n";

const std::string response_head404 = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html; charset=utf-8\r\n\r\n";

const std::string root_dir = "../front_end/";

const std::string default_filename = "index.html";
const std::string not_found_filename = "404.html";
