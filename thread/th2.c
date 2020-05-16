

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE 100

void* func(void* data)
{
	int id;
	int i = 0;
	id = *((int*)data);

	int status = 100;

	for(i=0; i<10; i++)
	{
		if(i==5)
			pthread_exit((void*)status);
		printf("%d : %d\n", id, i);
		sleep(1);
	}
}

int main()
{
	pthread_t th;
	int th_id;
	int status = 0;
	int a = 1;
	int b = 100;

	char buf[BUFSIZE];

	// 쓰레드 생성 아규먼트로1을 넘긴다
	
	printf("%d\n", status);
	th_id = pthread_create(&th, NULL, func, (void*)&a);
	printf("%d\n", status);
	if(th_id < 0)
	{
		perror("thread create error : ");
		exit(0);
	}
/*
	while(1)
	{
		fgets(buf, BUFSIZE, stdin);
		printf("message : %s\n", buf);
		buf[strlen(buf)-1] = 0;

		if(strcmp(buf, "exit")==0)
			break;
	}
*/

	pthread_join(th, (void**)&status);
	printf("%d\n", status);
	return 0;
}
