

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUFSIZE 1024

void error_handling(char* msg);

int main(int argc, char** argv)
{
	int server_sock;
	int client_sock;

	char msg[BUFSIZE];
	int str_len;

	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;

	int client_addr_size;

	if(argc != 2)
	{
		printf("usage : %s <port>\n", argv[0]);
		exit(1);
	}

	/* 서버 소켓 생성 */
	server_sock = socket(PF_INET, SOCK_STREAM, 0);
	if(server_sock == -1)
		error_handling("socket() error");
	fputs("socket creation success.\n", stdout);

	/* 서버 주소 구조체 초기화 */
	memset(&server_addr, 0 , sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(atoi(argv[1]));

	/* 소켓에 주소 할당 */
	if(bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr))==-1)
		error_handling("bind() error");
	fputs("bind success.\n", stdout);

	/* 연결 요청 대기 상태로 진입 */
	if(listen(server_sock, 5) == -1)
		error_handling("listen() error");
	fputs("listen success.\n", stdout);

	client_addr_size = sizeof(client_addr);

	/* 연결 요청 수락 */
	client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_addr_size);
	if(client_sock == -1)
		error_handling("accept() error");
	fputs("connection success.\n", stdout);


	/* 데이터 수신 및 전송 */
	/* 클라이언트가 EOF를 보낼 때까지 */
	/* 데이터를 받아서 서버 콘솔에 한 번 */
	/* 출력해 주고 클라이언트로 재전송 */
	while((str_len = read(client_sock, msg, BUFSIZE))!=0)
	{
		write(1, msg, str_len);
		write(client_sock, msg, str_len);
	}

	close(client_sock);
	return 0;
}

void error_handling(char* msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}




