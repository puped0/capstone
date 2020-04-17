



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
	int sock;

	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;

	int client_addr_size;
}socketdata;

typedef struct tagargument
{
	int index;
	char* line;
}argument;

void error_handling(char* msg);
void init(socketdata* sd);
void readmsg(socketdata* sd, char* msg);
int maketoken(char* msg, char** token);

void* speaking(void* data);

pthread_t speaker_thread[10];
int thread_state[10];

int main()
{
	socketdata sd;
	init(&sd);

	char msg[BUFSIZE];
	char* token[10];

	while(1)
	{
		readmsg(&sd, msg);
		maketoken(msg, token);
		
		// 연결 확인 응답
		if(atoi(token[0]) == 1)
		{
			sendto(sd.sock, "0", strlen("0")+1, 0, (struct sockaddr*)&(sd.client_addr), sd.client_addr_size);
		}
		// 대사 받아서 쓰레드 생성한 뒤 재생
		else if(atoi(token[0]) == 2)
		{
			argument th_arg;
			int index;

			while(1)
			{
				if(thread_state[index] == 0)
					break;
				index++;
			}

			th_arg.index = index;
			th_arg.line = token[1];
			
			thread_state[index] = pthread_create(&(speaker_thread[index]), NULL,  , (void*)&th_arg);
			if(thread_state[index]
		}
		else if(atoi(token[0]) == 3){}
		else if(atoi(token[0]) == 4){}
		else
		{
			printf("receive : %s\n", msg);
			sendto(sd.sock, msg, strlen(msg)+1, 0, (struct sockaddr*)&(sd.client_addr), sd.client_addr_size);
		}
	}

	return 0;
}

void error_handling(char* msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

void init(socketdata* sd)
{
	sd->sock = socket(PF_INET, SOCK_DGRAM, 0);

	if(sd->sock == -1)
		error_handling("socket() error");

	memset(&(sd->server_addr), 0, sizeof(sd->server_addr));
	sd->server_addr.sin_family = AF_INET;
	sd->server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	sd->server_addr.sin_port = htons(10000);

	if(bind(sd->sock, (struct sockaddr*)&(sd->server_addr), sizeof(sd->server_addr)) == -1)
		error_handling("bind() error");

	sd->client_addr_size = sizeof(sd->client_addr);

	/////////////////////////////////////////////
	
	memset(speaker_thread, 0, sizeof(speaker_thread));
	memset(thread_state, 0, sizeof(thread_state));
}

void readmsg(socketdata* sd, char* msg)
{
	memset(msg, 0, BUFSIZE);

	recvfrom(sd->sock, msg, BUFSIZE, 0, (struct sockaddr*)&(sd->client_addr), &(sd->client_addr_size));
}

int maketoken(char* msg, char** token)
{
	int i = 0;
	char* ptr = strtok(msg, " ");

	while(ptr != NULL)
	{
		token[i] = ptr;
		ptr = strtok(NULL, " ");
		i++;
	}
	return i;
}

void* speaking(void* data)
{

}
