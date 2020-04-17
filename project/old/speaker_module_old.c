
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUFSIZE 1024

void error_handling(char* msg);

int main()
{

	int sock;
	int client_addr_size;

	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;

	char buff_rcv[BUFSIZE];
	char buff_snd[BUFSIZE];

	sock = socket(PF_INET, SOCK_DGRAM, 0);

	if(sock == -1)
		error_handling("socket() error");

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(10000);

	if(bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
		error_handling("bind() error");

	while(1)
	{
		client_addr_size = sizeof(client_addr);
		recvfrom(sock, buff_rcv, BUFSIZE, 0, (struct sockaddr*)&client_addr, &client_addr_size);
		printf("receive : %s\n", buff_rcv);

		sprintf(buff_snd, "%s%s", buff_rcv, buff_rcv);
		sendto(sock, buff_snd, strlen(buff_snd)+1, 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
	}
}

void error_handling(char* msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
