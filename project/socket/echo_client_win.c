#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#pragma comment (lib,"ws2_32.lib")

#define BUFSIZE 512


void err_display(char *msg);
void err_quit(char *msg);
int recvn(SOCKET s, char* buf,int len,int flags);

int main(int argc, char * argv[])
{
	SOCKET sock;
	SOCKADDR_IN serveraddr;

	int retval;
	char buf[BUFSIZE+1];
	int len;
	WSADATA wsa;
	
	if(argc != 3 )
	{
		printf("Usage %s <IP> <port> \n", argv[0]);
		exit(1);
	}

	
	if(WSAStartup(MAKEWORD(2,2),&wsa) != 0)	
		return -1;

	sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock == INVALID_SOCKET) err_quit("sock()");
	
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
	serveraddr.sin_port = htons(atoi(argv[2]));
	
	retval = connect(sock,(SOCKADDR*)&serveraddr,sizeof(serveraddr));
	if(retval == SOCKET_ERROR) err_quit("connect()");

	while(1)
	{

		ZeroMemory(buf,sizeof(buf));
		printf("send To server : ");
		if(fgets(buf,BUFSIZE+1,stdin)==NULL)
			break;


		len = strlen(buf);
		if(buf[len-1] == '\n')
		{
			buf[len-1] = '\n';
			buf[len] = '\0';
		}
		if(strlen(buf)==0)
			break;


		retval = send(sock,buf,strlen(buf),0);
		if(retval == SOCKET_ERROR)
		{
			err_display("send()");
			break;
		}
		printf("recv len %d \n",retval);

	
		retval = recvn(sock,buf,retval,0);
		if(retval==SOCKET_ERROR)
		{
			err_display("recv()");
			break;
		}
		else if(retval==0)
			break;

	
		buf[retval] = '\0';
		printf("data:%s  len%d \n", buf, retval);
		
	}

	closesocket(sock);
	WSACleanup();

	return 0;
}

int recvn(SOCKET s, char* buf,int len,int flags)
{
	int received;
	char* ptr = buf;
	int left = len;
	

	while(left > 0)
	{
		received = recv(s,ptr,left,flags);
		if(received == SOCKET_ERROR)
			return SOCKET_ERROR;
		else if(received == 0)
			break;
		left -= received;
		ptr += received;
	}
	return (len-left);
}

void err_display(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,0,NULL);
	printf("[%s] %s",msg,(LPTSTR)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

void err_quit(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,0,NULL);
	MessageBox(NULL,(LPTSTR)lpMsgBuf,msg,MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(-1);
}
