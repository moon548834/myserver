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


int main(int argc, char *argv[]) {
	int sock;

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = INADDR_ANY;
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("create sock failed");
		exit(-1);
	}
	if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)	 {
		perror("connect");
		exit(-1);
	}
	int sock_stdin = fileno(stdin);
	int max_fd = sock;
	if (sock_stdin > sock) max_fd = sock_stdin;
	fd_set client_set;
	char writebuffer[1024];
	char readbuffer[1024];
	memset(writebuffer, 0, sizeof(writebuffer));
	memset(readbuffer, 0, sizeof(readbuffer));
	while (1) {
		FD_ZERO(&client_set);
		FD_SET(sock, &client_set);
		FD_SET(sock_stdin, &client_set);
		int ret = select(max_fd + 1, &client_set, NULL, NULL, NULL);
		if (ret < 0) {
			perror("seletct");
			exit(-1);
		}
		else if (ret == 0) {
			printf("timeout");
			continue;
		}
		else {
			if (FD_ISSET(sock, &client_set)) {
				int read_ret = read(sock, readbuffer, sizeof(readbuffer));
				if (read_ret == 0) {
					break;
				}
				else if(read_ret < 0)  {
					perror("read");
					exit(-1);
				}
				printf("%s",readbuffer);
				memset(readbuffer, 0, sizeof(readbuffer));
			}
			else if (FD_ISSET(sock_stdin, &client_set)) {
				read(sock_stdin, writebuffer, sizeof(writebuffer));
				printf("stdin: %s#", writebuffer);
				int write_ret = write(sock, writebuffer, strlen(writebuffer));
				if (write_ret == 0) {
					break;
				}
				else if (write_ret < 0) {
					perror("write");
					break;
				}
				memset(writebuffer, 0, sizeof(writebuffer));
			}
		}
	}
}
