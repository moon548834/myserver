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

#define MAX_EVENT_NUMBER 10

void setnonblocking(int fd) {
	int old_option = fcntl(fd, F_GETFL);
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_option);
	return;
}

void addfd2epoll(int epollfd, int fd) {
	setnonblocking(fd);
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET;
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
}


int main(int argc, char *argv[]) {
	if (argc <= 2) {
		printf("usage [ip_addr] [port]");
		return 0;
	}
	int port = atoi(argv[2]);
	int ret;
	vector<>
	sockaddr_in address;
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	inet_pton(AF_INET, (const char*)argv[1],&address.sin_addr);
	int listenfd = socket(PF_INET, SOCK_STREAM, 0);
	assert(listenfd != -1);
	assert(epollfd != -1);
	ret = bind(listenfd, (sockaddr*)&address, sizeof(address));
	assert(ret != -1);
	ret = listen(listenfd, 5);
	assert(ret != -1);
	int epollfd = epoll_create(5);
	assert(epollfd != -1);
	addfd2epoll(epollfd, listenfd);
	epoll_event events[MAX_EVENT_NUMBER];
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
			}
		}
	}
}
