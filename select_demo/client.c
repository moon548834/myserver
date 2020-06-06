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
	int max_fd = sock + 1;
	fd_set client_set;
	char writebuffer[1024];
	char readbuffer[1024];
	memset(writebuffer, 0, sizeof(writebuffer));
	strcpy(writebuffer, "test");
	memset(readbuffer, 0, sizeof(readbuffer));
	while (1) {
		FD_ZERO(&client_set);
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
				puts(readbuffer);
				memset(readbuffer, 0, sizeof(readbuffer));
			}
			else {
				sleep(2);
				int write_ret = write(sock, writebuffer, strlen(writebuffer));
				if (write_ret == 0) {
					break;
				}
				else if (write_ret < 0) {
					perror("write");
					break;
				}
			}
		}
	}
}
