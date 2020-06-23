#include "http_conn.h"

int HttpConn::m_epollfd = -1;

int setnonblocking( int fd ) {
    int old_option = fcntl( fd, F_GETFL );
    int new_option = old_option | O_NONBLOCK;
    fcntl( fd, F_SETFL, new_option );
    return old_option;
}

void addfd( int epollfd, int fd, bool one_shot ) {
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    if( one_shot ) {
        event.events |= EPOLLONESHOT;
    }
    epoll_ctl( epollfd, EPOLL_CTL_ADD, fd, &event );
    setnonblocking( fd );
}

void removefd( int epollfd, int fd ) {
    epoll_ctl( epollfd, EPOLL_CTL_DEL, fd, 0 );
    close( fd );
}


void modfd( int epollfd, int fd, int ev ) {
    epoll_event event;
    event.data.fd = fd;
    event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    epoll_ctl( epollfd, EPOLL_CTL_MOD, fd, &event );
}


HttpConn::HttpConn() {
}

HttpConn::~HttpConn() {
}

void HttpConn::init(sockaddr_in address, int connfd) {
	this->address = address;
	this->connfd = connfd;
	read_buf.clear();
	write_buf.clear();
	addfd(m_epollfd, connfd, true);
}

void HttpConn::close_conn() {
	if (connfd != -1) removefd(m_epollfd, connfd);
	connfd = -1;
}

void HttpConn::handle(){
	int read_ret = handle_readreq();
	if (read_ret == -1) {
		modfd(m_epollfd, connfd, EPOLLIN);
		close_conn();
	}
	else {
		modfd(m_epollfd, connfd, EPOLLOUT);
	}
}

int HttpConn::handle_readreq() {
	if (read_buf.size() == 0) return -1;
	std::regex split_re("\r\n");
	std::vector<std::string> request_message(std::sregex_token_iterator(read_buf.begin(), read_buf.end(), split_re, -1), std::sregex_token_iterator());
	std::regex split_line(" ");
	std::vector<std::string> request_first_line(std::sregex_token_iterator(request_message[0].begin(), request_message[0].end(), split_line, -1), std::sregex_token_iterator());
	if (request_first_line.size() == 0) return -1;
	std::string filename = request_first_line[1];
	write_buf = get_http_response(request_first_line[1]);
	return 0;
}

std::string HttpConn:: get_http_response(std::string filename) {
	std::cout << filename << std::endl;
	if (filename == "/" || filename == "\\") filename = default_filename;
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

std::string HttpConn:: get_file_content(std::string filedir) {
	//firedir has confirmed to be available
	std::ifstream in;
	in.open(filedir);
	std::stringstream strStream;
	strStream << in.rdbuf();
	in.close();
    return(strStream.str());
}

void HttpConn::write() {
	const char *write_buf_ptr = write_buf.c_str();
	int writes_size = write_buf.size();
	int bytes_write = 0;
	while(writes_size > 0) {
		if ((bytes_write = send(connfd, write_buf_ptr, writes_size, 0)) <= 0) {
			if (bytes_write < 0) {
				if (errno == EAGAIN || errno == EWOULDBLOCK) {
					break;
				}
				else if (errno == EINTR) continue;
				else  {
					close_conn();
					break;
				}
			}
		}
		else if (bytes_write == 0) {
			close_conn();
			break;
		}
		write_buf_ptr += bytes_write;
		writes_size -= bytes_write;
	}
	close_conn();
}

int HttpConn::read() {
	std::vector<char> buffer(BUFFER_SIZE);
	while (true) {
		int bytes_read = recv(connfd, &buffer[0], BUFFER_SIZE, 0);
		if (bytes_read < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				break;
			}
			else if(errno == EINTR)	continue;
			else {
				close_conn();
				break;
			}
		}
		else if (bytes_read == 0) {
			close_conn();
		}
		read_buf.append(buffer.cbegin(), buffer.cend());
	}
	return read_buf.size();
}

