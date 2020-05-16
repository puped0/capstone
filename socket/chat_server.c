

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUFSIZE 1024

void error_handling(char* msg);
void* sendmsg_th(void* data);

int thread_stat = 0;

int main(int argc, char** argv)
{
	int server_sock;
	int client_sock;

	char msg[BUFSIZE];
	int str_len;

	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;

	int client_addr_size;

	pthread_t th;
	int th_id;
	int status;


	if(argc != 2)
	{
		printf("usage : %s <port>\n", argv[0]);
		exit(1);
	}

	/* 서버 소켓 생성*/
	server_sock = socket(PF_INET, SOCK_STREAM, 0);
	if(server_sock == -1)
		error_handling("socket() error");
	fputs("socket creation success.\n", stdout);

	/* 서버 주소 구조체 초기화*/
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port =  htons(atoi(argv[1]));

	/* 소켓에 주소 할당*/
	if(bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr))==1)
		error_handling("bind() error");
	fputs("bind success.\n", stdout);

	/* 연결요청 대기상대로 진입*/
	if(listen(server_sock, 5) == -1)
		error_handling("listen() error");
	fputs("listen success.\n", stdout);

	client_addr_size = sizeof(client_addr);

	client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_addr_size);
	if(client_sock == -1)
		error_handling("accept() error");
	fputs("connection success.\n", stdout);

	th_id = pthread_create(&th, NULL, sendmsg_th, (void*)&client_sock);
	if(th_id < 0)
	{
		perror("thread create error : ");
		exit(1);
	}

	while(1)
	{
		str_len = read(client_sock, msg, BUFSIZE);
	//	fputs("전달받은 메시지 : ", stdout);
		fputs(msg, stdout);
		fputc('\n', stdout);
		if(strcmp(msg, "q")==0 || thread_stat == 1)
			break;

	}

	pthread_join(th, (void**)&status);
	
	close(client_sock);
	return 0;
}

void* sendmsg_th(void* data)
{
	int str_len;
	int client_sock = *((int*)data);
	char msg[BUFSIZE];

	while(1)
	{
	//	fputs("전송할 메시지를 입력하세요(q to quit) : ", stdout);
		fgets(msg, BUFSIZE, stdin);

		str_len = strlen(msg);

		msg[str_len - 1]=0;

		write(client_sock, msg, str_len);

		if(strcmp(msg, "q")==0)
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
