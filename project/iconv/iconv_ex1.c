
#include <stdio.h>
#include <string.h>
#include <iconv.h>

int changecharset(char* szsrccharset, char* szdstcharset, char* szsrc, int nsrclength, char* szdst, int ndstlength)
{
	int result;
	size_t nsrcstrlen; 
	size_t ndststrlen;
	size_t cc; 

	iconv_t it = iconv_open(szdstcharset, szsrccharset);
	if(it==(iconv_t)(-1))
		return 0;

	result = 1;
	nsrcstrlen = nsrclength;
	ndststrlen = ndstlength;
	cc = iconv(it, &szsrc, &nsrcstrlen, &szdst, &ndststrlen);

	if(cc = (size_t)(-1))
		result = 0;

	if(iconv_close(it)==-1)
		result = 0;
	

	return result;
}

int main()
{
	char szsrc[] = "대한민국";
	char szdst[100];
	changecharset("utf-8", "euc-kr", szsrc, strlen(szsrc), szdst, sizeof(szdst));
	printf("%s\n", szdst);
	return 0;
}
