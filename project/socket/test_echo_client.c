

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUFSIZE 1024

void error_handling(char *msg);

int main(int argc, char** argv)
{
	int server_sock;

	struct sockaddr_in server_addr;

	char msg[BUFSIZE];

	int str_len;

	while(1)
	{
		server_sock = socket(PF_INET, SOCK_STREAM, 0);
	
		if(server_sock == -1)
			error_handling("socket() error");
	
		memset(&server_addr, 0, sizeof(server_addr));
		server_addr.sin_family = AF_INET;
		server_addr.sin_addr.s_addr = inet_addr("192.168.0.142");
		server_addr.sin_port = htons(10000);
		if(connect(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr))==-1)
			error_handling("connect() error");


		fputs("메시지 : ", stdout);
		fgets(msg, BUFSIZE, stdin);
		write(server_sock, msg, strlen(msg));

		str_len = read(server_sock, msg, BUFSIZE-1);
		msg[str_len] = 0;
		printf("도착 : %s\n", msg);
		
		close(server_sock);
	}
	return 0;
}

void error_handling(char* msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);

	exit(1);
}
