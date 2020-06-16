#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <vector>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#define MAX_CLIENT_NUMBER 5
#define MAX_EVENT_NUMBER 8
#define BUFFER_SIZE 1024

struct client_data {
	sockaddr_in address;
	int pipefd[2];
	int connfd;
	pid_t pid;
};

std::vector<client_data> clients(MAX_CLIENT_NUMBER);
std::vector<int> client_pids(MAX_CLIENT_NUMBER);
int client_cnt;
int sig_pipefd[2];
int listenfd;
int epollfd;
int shmfd;
bool stop_server;
bool stop_child = false;
const char shm_name[20] = "/myshm";
char *shared_mem;

void setnonblocking(int fd) {
	int old_option = fcntl(fd, F_GETFL);
	int new_option = old_option | O_NONBLOCK ;
	fcntl(fd, F_SETFL, new_option);
	return;
}

void signal_handler(int sig) {
	int save_errno = errno;
	send(sig_pipefd[1], (char*)&sig, 1, 0);
	errno = save_errno;
}

void child_term_handler(int ) {
	stop_child = true;
}

void addsig(int sig, void(*signal_handler)(int)) {
	struct sigaction sa;
	memset(&sa, '\0', sizeof(sa));
	sa.sa_handler = signal_handler;
	sigfillset(&sa.sa_mask);	
	assert(sigaction(sig, &sa, NULL) != -1);
}

void addfd2epoll(int epollfd, int fd){
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET;
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
	setnonblocking(fd);
}

void run_child(client_data client) {
	epoll_event events[MAX_EVENT_NUMBER];
	int child_epollfd = epoll_create(5);
	assert(child_epollfd != -1);
	int connfd = client.connfd;
	int pipefd = client.pipefd[1];
	addfd2epoll(child_epollfd, connfd);
	addfd2epoll(child_epollfd, pipefd);
	// remind to add sig when stop
	addsig(SIGTERM, child_term_handler);
	int ret;
	while (!stop_child) {
		int number = epoll_wait(child_epollfd, events, MAX_EVENT_NUMBER, -1);
		for (int i = 0; i < number; i++) {
			if (events[i].data.fd == connfd && events[i].events & EPOLLIN) {
				memset(shared_mem + client_cnt*BUFFER_SIZE, '\0', BUFFER_SIZE);
				ret = recv(connfd, shared_mem + client_cnt * BUFFER_SIZE, BUFFER_SIZE - 1, 0);
				// message recieved from users
				if (ret < 0) {
					if(errno != EAGAIN) stop_child = true;
				}
				else if(ret == 0) {
					stop_child = true;
				}
				else {
					ret = send(pipefd, (char*)&client_cnt, sizeof(client_cnt), 0);
					assert(ret != -1);
				}

			}
			else if (events[i].data.fd == client.pipefd[1] && (events[i].events & EPOLLIN)) {
				// recv message from father, which user have updated shared memory
				int client_id = -1;
				ret = recv(pipefd, &client_id, sizeof(client_id), 0);
				if (ret < 0) {
					if (errno != EAGAIN) {
						stop_child = true;
					}
				}
				else if (ret == 0) {
					stop_child = true;
				}
				else {
					send(connfd, shared_mem + client_id * BUFFER_SIZE, BUFFER_SIZE, 0);
				}
			}
			else {
				continue;
			}
		}
	}
	close(connfd);
	close(pipefd);
	close(child_epollfd);
	return;
}

int main(int argc, char *argv[]) {
	if (argc <= 2) {
		printf("usage [ip address] [port]\n");
		return 0;
	}
	int port_number = atoi(argv[2]);
	int ret;
	sockaddr_in address;
	bzero(&address, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(port_number);
	inet_pton(AF_INET, (const char*)argv[1], &address.sin_addr);
	int listenfd = socket(PF_INET, SOCK_STREAM, 0);
	assert(listenfd != -1);
	ret = bind(listenfd, (sockaddr*)&address,sizeof(address));
	assert(ret != -1);
	ret = listen(listenfd, 5);
	assert(ret != -1);
	int epollfd = epoll_create(5);
	assert(epollfd != -1);
	addfd2epoll(epollfd, listenfd);

	client_cnt = 0;
	for (int i = 0; i < MAX_CLIENT_NUMBER; i++) {
		client_pids[i] = -1;
	}
	ret = socketpair(PF_UNIX, SOCK_STREAM, 0, sig_pipefd);
	assert(ret != -1);
	addfd2epoll(epollfd, sig_pipefd[0]);
	setnonblocking(sig_pipefd[1]);
	addsig(SIGCHLD, signal_handler);
	addsig(SIGTERM, signal_handler);
	addsig(SIGINT, signal_handler);
	shmfd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
	assert(shmfd != -1);
	ret = ftruncate(shmfd, MAX_CLIENT_NUMBER * BUFFER_SIZE);
	assert(ret != -1);
	shared_mem = (char*)mmap(NULL, MAX_CLIENT_NUMBER * BUFFER_SIZE,PROT_READ | PROT_WRITE, MAP_SHARED,shmfd, 0);
	epoll_event events[MAX_EVENT_NUMBER];
	while(!stop_server) {
		int number = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
		if ((number < 0) && (errno != EINTR)) {
			printf("epoll failed\n");
			break;
		}
		for (int i = 0; i < number; i++) {
			if(events[i].data.fd == listenfd) {
				struct sockaddr_in address;
				bzero(&address, sizeof(address));
				socklen_t address_len = sizeof(address);
				int connfd = accept(listenfd, (struct sockaddr*)&address, &address_len);
				assert(connfd != -1);
				clients[client_cnt].connfd = connfd;
				clients[client_cnt].address = address;
				ret = socketpair(PF_UNIX, SOCK_STREAM, 0, clients[client_cnt].pipefd);
				assert(ret != -1);
				pid_t pid = fork();
				if (pid < 0) {
					printf("fork failed\n");
					exit(0);
				}
				if (pid == 0) {
					//child
					close(epollfd);
					close(listenfd);
					close(clients[client_cnt].pipefd[0]);
					close(sig_pipefd[0]);
					close(sig_pipefd[1]);
					run_child(clients[client_cnt]);
					munmap(shared_mem, client_cnt * BUFFER_SIZE);
					exit(0);
				}
				else {
					close(connfd);
					close(clients[client_cnt].pipefd[1]);
					addfd2epoll(epollfd, clients[client_cnt].pipefd[0]);
					client_cnt++;	
				}
			}
			else if(events[i].data.fd == sig_pipefd[0] && (events[i].events & EPOLLIN)) { //this signal is automatically sent by child when it exits :)
					exit(0);
			}
			else if (events[i].events & EPOLLIN){ 
				// message(only tell which user change, not whole)
				int client_id = 0;
				ret = recv(events[i].data.fd, &client_id, sizeof(client_id), 0);
				printf("father should recieve req\n");
				if (ret <= 0) {
					printf("why?\n");
					continue;
				}
				else {
					for (int j = 0; j < client_cnt; j++) {
						if (clients[j].pipefd[0] != events[i].data.fd) {
							send(clients[j].pipefd[0], (char*)&client_id, sizeof(client_id), 0);
						}
					}
				}
			}
		}
	}
	return 0;
}
