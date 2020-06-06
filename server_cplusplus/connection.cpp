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
		perror("socket");
		exit(-1);
	}
	// handle bind failed
	int val = 1;
	int res = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void *)&val, sizeof(int));
	if (res == -1) {
		perror("setsockopt");
		exit(-1);
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
		std::regex split_body("\r\n");
		std::vector<std::string> request_message(std::sregex_token_iterator(buf.begin(), buf.end(), split_body, -1), std::sregex_token_iterator());
		std::regex split_line(" ");
		if (request_message.size() == 0) LOG("request_message is empty!");
		std::vector<std::string> request_first_line(std::sregex_token_iterator(request_message[0].begin(), request_message[0].end(), split_line, -1), std::sregex_token_iterator());
		//std::cout << request_first_line[1] << std::endl;
		if (request_first_line.size() == 0) LOG("first line is empty!");
		std::string filename = request_first_line[1];
		std::string message = get_http_response(request_first_line[1]);
		write(sock, message.c_str(), message.size());
	}
	delete(char_buf);
}

std::string Connection:: get_http_response(std::string filename) {
	//COUT(filename);
	if (filename == "/") filename = default_filename;
	std::string dir = root_dir + filename;
	LOG(dir);
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
	std::ifstream in;
	in.open(filedir);
	std::stringstream strStream;
	strStream << in.rdbuf();
	in.close();
    return(strStream.str());
}



