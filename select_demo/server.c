#include <stdio.h>
#include <netinet/in.h>   
#include <sys/types.h>      
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/select.h>
#include <string.h>

#define BUFF_SIZE 1024
#define BACKLOG  7
#define PORT 9999
#define CLIENT_NUM 3

int client_fds[CLIENT_NUM];

int main(int argc, char *argv[]) {
	int server_sock;

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = INADDR_ANY;
	if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("create sock failed");
		exit(-1);
	}
	if (bind(server_sock, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
		perror("bind failed");
		exit(-1);
	}
	if (listen(server_sock, BACKLOG) < 0) {
		perror("listen failed");
		exit(-1);
	}
	fd_set server_sock_set;
	int max_fd = 1;
	struct timeval timeout;
	printf("waiting for client\n");
	for (int i = 0; i < CLIENT_NUM; i++) client_fds[i] = -1;
	while (1) {
		timeout.tv_sec = 10;
		timeout.tv_usec = 0;
		FD_ZERO(&server_sock_set);
		FD_SET(server_sock, &server_sock_set);
		if (max_fd < server_sock) max_fd = server_sock;
		for (int i = 0; i < CLIENT_NUM; i++) {
			if (client_fds[i] != -1) {
				FD_SET(client_fds[i], &server_sock_set);
				if (max_fd < client_fds[i]) max_fd = client_fds[i];
			}
		}
		int ret = select(max_fd + 1, &server_sock_set, NULL, NULL, &timeout);
		if (ret < 0) {
			perror("select failed");
			continue;
		}
		else if (ret == 0) {
			printf("time out\n");
			continue;
		}
		else {
			if (FD_ISSET(server_sock, &server_sock_set)) {
				struct sockaddr_in client_addr;
				socklen_t client_addr_len;
				int client_sock;
REPEAT_ACCEPT:
				client_sock = accept(server_sock, (struct sockaddr*) &client_addr, &client_addr_len);
				if (client_sock == -1) {
					if (errno == EINTR) goto REPEAT_ACCEPT;
					else {
						perror("accept failed");
						exit(-1);
					}
				}
				printf("accept client: %d\n", client_sock);
				int i = 0;
				for (; i < CLIENT_NUM; i++) {
					if (client_fds[i] < 0) {
						client_fds[i] = client_sock;  
						break;
					}
				}
				if (i == CLIENT_NUM) printf("too many clients:(\n");
			}
			else {
				int i = 0;
				char *buffer = (char *)malloc(BUFF_SIZE * sizeof(char));
				for (; i < CLIENT_NUM; i++) {
					int client_sock = client_fds[i];
					if (client_sock < 0) continue;
					if (FD_ISSET(client_sock, &server_sock_set)) {
						int ret = read(client_sock, buffer, BUFF_SIZE);
						if (ret < 0) {
							perror("read");
							exit(-1);
						}
						else if (ret == 0) { // client closed
							FD_CLR(client_sock, &server_sock_set);
							close(client_sock);
							client_fds[i] = -1;
						}
						else { 
							printf("message recieved %s\n", buffer);
							write(client_sock, buffer, strlen(buffer) + 1);
						}
					}
				}
				free(buffer);
			}
		}
	}
}

