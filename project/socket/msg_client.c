

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUFSIZE 1024

void error_handling(char* message);
void* sendmsg_th(void* data);

int thread_stat = 0;

int main(int argc, char** argv)
{
	int server_sock;
	struct sockaddr_in server_addr;
	char msg[BUFSIZE];
	int str_len;

	pthread_t th;
	int th_id;
	int status;

	if(argc != 3)
	{
		printf("Usage : %s <IP> <PORT> \n", argv[0]);
		exit(1);
	}

	server_sock = socket(PF_INET, SOCK_STREAM, 0);

	if(server_sock == -1)
		error_handling("socket() error");

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	server_addr.sin_port = htons(atoi(argv[2]));
	if(connect(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr))== -1)
		error_handling("connect() error");

	th_id = pthread_create(&th, NULL, sendmsg_th, (void*)&server_sock);
	if(th_id < 0)
	{
		perror("thread create error : ");
		exit(1);
	}


	while(1)
	{
		str_len = read(server_sock, msg, BUFSIZE);
//		fputs("전달받은 메시지 : ", stdout);
		fputs(msg, stdout);
		fputc('\n', stdout);
		if(strcmp(msg, "q")==0 || thread_stat==1)
			break;
	}

	pthread_join(th, (void**)&status);

	close(server_sock);
	return 0;
}

void* sendmsg_th(void* data)
{
	int str_len;
	int server_sock = *((int*)data);
	char msg[BUFSIZE];


	while(1)
	{
//		fputs("전송할 메시지를 입력하세요(q to quit) : ", stdout);
		fgets(msg, BUFSIZE, stdin);

		str_len = strlen(msg);

		msg[str_len - 1] = 0;

		write(server_sock, msg, str_len);

		if(strcmp(msg, "q") == 0)
		{
			thread_stat = 1;
			break;
		}
	}
}

void error_handling(char* msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
