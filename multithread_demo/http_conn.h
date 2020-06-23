#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
#include <string>
#include <memory.h>
#include <vector>
#include <regex>
#include <fstream>
#include <boost/filesystem.hpp>
#include <sys/epoll.h>
#include <fcntl.h>
#include <iostream>

#define BUFFER_SIZE 2048
class HttpConn {
	public:
		static int m_epollfd;
		HttpConn();
		~HttpConn();
		void close_conn();
		void handle();
		void init(sockaddr_in address, int connfd);
		int read();
		void write();
		
	private:
		std::string read_buf;
		std::string write_buf;
		sockaddr_in address;
		int connfd;
		int handle_readreq();
		int handle_writereq();
		std::string get_http_response(std::string filename);	
		std::string get_file_content(std::string filedir);
};

const std::string http_message_head_200= 
	"HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\n\r\n";

const std::string http_message_head_404= 
	"HTTP/1.1 404 Not Found\r\nContent-Type: text/html; charset=utf-8\r\n\r\n";

const std::string root_dir = "/root/work/myserver/front_end/";

const std::string default_filename = "index.html";
const std::string not_found_filename = "404.html";

