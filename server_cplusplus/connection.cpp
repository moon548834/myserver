#include "connection.h"

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

void Connection:: http_handler() {
	char *char_buf = new char[3000];
	int rec_message_cnt = read(sock, char_buf, 2048);
	if (rec_message_cnt != 0) {
		std::string buf(char_buf);
		COUT(buf);
		std::regex split_body("\r\n");
		std::vector<std::string> request_message(std::sregex_token_iterator(buf.begin(), buf.end(), split_body, -1), std::sregex_token_iterator());
		std::regex split_line(" ");
		std::vector<std::string> request_first_line(std::sregex_token_iterator(request_message[0].begin(), request_message[0].end(), split_line, -1), std::sregex_token_iterator());
		//std::cout << request_first_line[1] << std::endl;
		std::string filename = request_first_line[1];
		std::string message = get_http_response(request_first_line[1]);
		std::cout << message << std::endl;
		write(sock, message.c_str(), message.size());
	}
}

std::string Connection:: get_http_response(std::string filename) {
	COUT(filename);
	if (filename == "/") filename = default_filename;
	std::string dir = root_dir + filename;
	std::string http_message_head;
	std::string http_message_body;
	if (boost::filesystem::exists(dir)) {
		http_message_head = http_message_head_200;
		http_message_body =	get_file_content(dir);
	}
	else {
		dir = root_dir + not_found_filename;
		http_message_head = http_message_head_404;
		http_message_body =	get_file_content(dir);
	}
	return  http_message_head + http_message_body;
}

std::string Connection:: get_file_content(std::string filedir) {
	//firedir has confirmed to be available
	//a fast way for reading file
	//according to this link: 
	//http://insanecoding.blogspot.com/2011/11/how-to-read-in-file-in-c.html
	std::ifstream in;
	in.open(filedir, std::ios::in);
	std::string content;
	in.seekg(0, std::ios::end);
	content.resize(in.tellg());
    in.seekg(0, std::ios::beg);
    in.read(&content[0], content.size());
    in.close();
    return(content);
}



