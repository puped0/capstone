

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#define  BUFF_SIZE   1024

int main()
{
	// 순서
	// <스피커 재생 -> 대본 대사> 순서로 진행
	// 첫 순서의 경우 재생할 스피커가 없으므로 아무것도
	// 안보내고 대사만  전달함
	// 대사를 전송한 뒤엔 스피커가 대본을 모두 읽기를 
	// 기다림
	// 기다리는건 소켓에 특정 문자열이
	// 대사를 읽는 스피커 만큼
	// (스피커의 갯수던 대사 갯수던)
	// 세서 대사를 다 읽기를 기다림
	// 다 읽었으면 스피커를 재생시키고 대사 전송 반복
	
	int sock;
	int server_addr_size;

	struct sockaddr_in   server_addr;

	char buff_snd[BUFF_SIZE+5]; 	
	char buff_rcv[BUFF_SIZE+5];

	sock  = socket( PF_INET, SOCK_DGRAM, 0);

	if( -1 == sock)
	{
		printf( "socket 생성 실패n");
		exit( 1);
	}

	memset( &server_addr, 0, sizeof( server_addr));
	server_addr.sin_family     = AF_INET;
	server_addr.sin_port       = htons(10001);
	server_addr.sin_addr.s_addr= inet_addr("127.0.0.1");

	while(1)
	{
		fgets(buff_snd, BUFF_SIZE, stdin);
		buff_snd[strlen(buff_snd)-1] = 0;
			
		sendto( sock, buff_snd, strlen(buff_snd)+1, 0, (struct sockaddr*)&server_addr, sizeof( server_addr));
		if(strcmp(buff_snd, "100")==0)
			break;	
	}

	close( sock);

	return 0;
}
