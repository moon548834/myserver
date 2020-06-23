#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <memory.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <pthread.h>
#include <vector>
#include "http_conn.h"
#include "thread_pool.h"
#include "locker.h"

#define MAX_EVENT_NUMBER 10
#define MAX_CLIENT_FD	1024

extern int addfd( int epollfd, int fd, bool one_shot );
extern int removefd( int epollfd, int fd );

int main(int argc, char *argv[]) {
	if (argc <= 2) {
		printf("usage [ip_addr] [port]\n");
		return 0;
	}
	int port = atoi(argv[2]);
	int ret;
	ThreadPool<HttpConn> *http_thread_pool = new ThreadPool<HttpConn>(4);
	std::vector<HttpConn> clients(MAX_CLIENT_FD);
	sockaddr_in address;
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = INADDR_ANY; 
	int listenfd = socket(PF_INET, SOCK_STREAM, 0);
	assert(listenfd != -1);
	ret = bind(listenfd, (sockaddr*)&address, sizeof(address));
	assert(ret != -1);
	ret = listen(listenfd, 5);
	assert(ret != -1);
	int epollfd = epoll_create(5);
	assert(epollfd != -1);
	addfd(epollfd, listenfd, false);
	epoll_event events[MAX_EVENT_NUMBER];
	HttpConn::m_epollfd = epollfd;
	while (true) {
		int number = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, 0);
		for (int i = 0; i < number; i++) {
			int curfd = events[i].data.fd;
			if(curfd == listenfd) {
				struct sockaddr_in newaddress;
				memset(&newaddress, 0, sizeof(newaddress));
				socklen_t newaddress_len = sizeof(newaddress);
				int connfd = accept(curfd, (struct sockaddr*)&newaddress, &newaddress_len);
				assert(connfd != -1);
				std::cout << "connfd:" << connfd << std::endl;
				clients[connfd].init(newaddress, connfd);
			}
			else if (events[i].events & EPOLLIN) {
				if (clients[curfd].read() > 0) http_thread_pool->add(&clients[curfd]);
			}
			else if (events[i].events & EPOLLRDHUP) {
				clients[curfd].close_conn();
			}
			else if (events[i].events & EPOLLOUT) {
				clients[curfd].write();
			}
		}
	}
	return 0;
}
