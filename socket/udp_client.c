

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUFSIZE 1024

void error_handling(char* msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}


int main(int argc, char** argv)
{
	int sock;
	int server_addr_size;

	struct sockaddr_in server_addr;

	char msg[BUFSIZE];
	printf("send : ");
	fgets(msg, BUFSIZE, stdin);

	sock = socket(PF_INET, SOCK_DGRAM, 0);

	if(sock == -1)
		error_handling("socket() error");

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_addr.sin_port = htons(10000);

	sendto(sock, msg, strlen(msg)+1, 0, (struct sockaddr*)&server_addr, sizeof(server_addr));

	server_addr_size = sizeof(server_addr);
	recvfrom(sock, msg, BUFSIZE, 0, (struct sockaddr*)&server_addr, &server_addr_size);

	printf("receive : %s\n", msg);
	close(sock);
	
	return 0;
}

