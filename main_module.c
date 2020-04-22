


#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUFSIZE 1024

typedef struct tagsocketdata
{
	int server_sock;
	int client_sock;

	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;

	int client_addr_size;
}socketdata;

void error_handling(char* msg);
void init();
void readmsg(char* msg);
int maketoken(char* msg, char** token);

socketdata sd;
socketdata speaker_sd[10];

int main()
{
	init();
	char msg[BUFSIZE];
	char* token[10];

	while(1)
	{
		// readmsg(msg);

		fgets(msg, BUFSIZE, stdin);
		
		maketoken(msg, token);

		// 스피커모듈 통신 확인
		if(atoi(token[0]) == 1)
		{
				
		}
		// 대본 시작
		else if(atoi(token[0]) == 2){}
		// 정지, 재생, 앞으로, 뒤로 등등...
		else if(atoi(token[0]) == 3){}
		else if(atoi(token[0]) == 4){}
		else{}

	}

	return 0;
}

void error_handling(char* msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

void init()
{
	sd.server_sock = socket(PF_INET, SOCK_STREAM, 0);
	if(sd.server_sock == -1)
		error_handling("socket() error");

	// int option = 1;
	// setsockopt(sd.server_sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

	memset(&(sd.server_addr), 0, sizeof(sd.server_addr));
	sd.server_addr.sin_family = AF_INET;
	sd.server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	sd.server_addr.sin_port = htons(10000);

	if(bind(sd.server_sock, (struct sockaddr*)&(sd.server_addr), sizeof(sd.server_addr)) == -1)
		error_handling("bind() error");

	if(listen(sd.server_sock, 5) == -1)
		error_handling("listen() error");

	sd.client_addr_size = sizeof(struct sockaddr_in);

	sd.client_sock = accept(sd.server_sock, (struct sockaddr*)&(sd.client_addr), &(sd.client_addr_size));
	if(sd.client_sock == -1)
		error_handling("accept() error");

	/*
	 * ip관련 xml파일을 읽고 읽은 ip로 udp 소켓을 만듬
	 */

	///////////////////////////
	// 192.168.0.138
	/*
	speaker_sd[0].server_sock = socket(PF_INET, SOCK_DGRAM, 0);
	if(speaker_sd[0].server_sock == -1)
		error_handling("socket() error(udp)");

	memset(&(speaker_sd[0].server_addr), 0, sizeof(struct sockaddr_in));
	speaker_sd[0].server_addr.sin_family = AF_INET;
	speaker_sd[0].server_addr.sin_addr.s_addr = inet_addr("192.168.0.138");
	speaker_sd[0].server_addr.sin_port = htons(10001);

	speaker_sd[0].client_addr_size = sizeof(struct sockaddr_in);
	*/
	///////////////////////////
		

}

void readmsg(char* msg)
{
	int str_len;

	str_len = read(sd.client_sock, msg, BUFSIZE);
	msg[str_len] = 0;
}

int maketoken(char* msg, char** token)
{
	int i = 0;
	
	// 구분 문자 " " 이외의 것을 사용할 것
	char* ptr = strtok(msg, ",");
	
	while(ptr != NULL)
	{
		token[i] = ptr;
		ptr = strtok(NULL, ",");
		i++;
	}
	
	return i;
}

