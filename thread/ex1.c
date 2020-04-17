

#include <stdio.h>
#include <string.h>

int main()
{
	int i;
	char buf[100];

	while(1)
	{
		fgets(buf, 100, stdin);
		printf("message : %s\n", buf);
		buf[strlen(buf)-1] = 0;
		
		if(strcmp(buf, "exit")==0)
			break;
	}
	
	return 0;
}
