


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUFSIZE 1024

int main()
{
	int send_sock;
	int send_addr_size;
	struct sockaddr_in send_addr;

	int recv_sock;
	int recv_addr_size;
	struct sockaddr_in recv_addr;

	char buf[BUFSIZE];

	recv_sock = socket(PF_INET, SOCK_DGRAM, 0);
	if(recv_sock == -1)
	{
		printf("socket 생성 실패\n");
		exit(1);
	}

	memset(&recv_addr, 0, sizeof(recv_addr));
	recv_addr.sin_family = AF_INET;
	recv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	recv_addr.sin_port = htons(10002);

	if(bind(recv_sock, (struct sockaddr*)&recv_addr, sizeof(recv_addr)) == -1)
	{
		printf("bind() 실행 에러\n");
		exit(1);
	}

	//////////////////////////////////////////////////

	send_sock = socket(PF_INET, SOCK_DGRAM, 0);

	if(send_sock == -1)
	{
		printf("socket 생성 실패\n");
		exit(1);
	}

	memset(&send_addr, 0, sizeof(struct sockaddr_in));
	send_addr.sin_family = AF_INET;
	send_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	send_addr.sin_port = htons(10001);

	/////////////////////////////////////////////////

	sendto(send_sock, "hello", strlen("hello")+1, 0, (struct sockaddr*)&send_addr, sizeof(struct sockaddr_in));

	recv_addr_size = sizeof(struct sockaddr_in);

	recvfrom(recv_sock, buf, BUFSIZE, 0, (struct sockaddr*)&recv_addr, &recv_addr_size);

	printf("%s\n", buf);

	close(send_sock);
	close(recv_sock);
	return 0;
}
