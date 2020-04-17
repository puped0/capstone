


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
void init(socketdata* sd);
void readmsg(socketdata* sd, char* msg);
int maketoken(char* msg, char** token);

int test(char* ip);

int main(int argc, char** argv)
{
	socketdata sd;
//	init(&sd);

	char msg[BUFSIZE];
	char* token[10];

	while(1)
	{	
		// ex) 1 192.168.0.138
		readmsg(&sd, msg);
		//fgets(msg, BUFSIZE, stdin); 
		maketoken(msg, token);
		

		/* 스피커모듈과 연결 확인*/
		if(atoi(token[0]) == 1) 
		{
			int success = test(token[1]);
			
			if(success == 1)
			//	printf("true\n");
				write(sd.client_sock, "true", sizeof("true")-1);
			else
			//	printf("false\n");
				write(sd.client_sock, "false", sizeof("false")-1);
		}
		/* 대본 시작 */
//		else if(atoi(token[0]) == 2)
//		else if(atoi(token[0]) == 3)
		else
		{
			write(sd.client_sock, msg, strlen(msg)-1);			
		}
		
		close(sd.client_sock);
		close(sd.server_sock);
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

}

void readmsg(socketdata* sd, char* msg)
{
	int server_sock;
	int client_sock;
	int option = 1;

	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;

	int client_addr_size;

	int str_len;
	
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(10000);

	server_sock = socket(PF_INET, SOCK_STREAM, 0);	

	if(server_sock == -1)
		error_handling("socket() error");
	setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
	if(bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
		error_handling("bind() error");
	
	if(listen(server_sock, 5) == -1)
		error_handling("listen() error");

	client_addr_size = sizeof(client_addr);
	
	client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_addr_size);
	if(client_sock == -1)
		error_handling("accept() error");
	fputs("connection success.\n", stdout);

	str_len = read(client_sock, msg, BUFSIZE);
	msg[str_len] = '\0';

	sd->server_sock = server_sock;
	sd->client_sock = client_sock;
	sd->server_addr = server_addr;
	sd->client_addr = client_addr;
}

int maketoken(char* msg, char** token)
{
	int i =0;
	char* ptr = strtok(msg, " ");

	while(ptr != NULL)
	{
		token[i] = ptr;
		ptr = strtok(NULL, " ");
		i++;
	}
	return i;
}

int test(char* ip)
{
	int server_sock;
	int server_addr_size;

	struct sockaddr_in server_addr;

	char msg[BUFSIZE];
	int success;

	server_sock = socket(PF_INET, SOCK_DGRAM, 0);

	if(server_sock == -1)
		error_handling("socket() error");

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip);
	server_addr.sin_port = htons(10000);

	success = sendto(server_sock, "true", strlen("true")+1, 0, (struct sockaddr*)&server_addr, sizeof(server_addr)); 

	if(success != -1)
	{
		server_addr_size = sizeof(server_addr);
		success = recvfrom(server_sock, msg, BUFSIZE, 0, (struct sockaddr*)&server_addr, &server_addr_size);
	}

	close(server_sock);

	if(success == -1)
		return 0;
	else
		return 1;
}

