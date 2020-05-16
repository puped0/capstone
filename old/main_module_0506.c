


#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#define BUFSIZE 1024

typedef struct tagsocketdata
{
	int server_sock;
	int client_sock;

	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;

	int client_addr_size;
}socketdata;

typedef struct tagcharacter
{
	char name[20];
	int gender;
	int age;
}character;

typedef struct tagdialogue
{
	int index;
	char actor[20];
	char line[500];
}dialogue;

typedef struct tagstory
{
	char title[30];

	double version;
	int numofactor;
	int numofdialogue;
	int numofline;

	character* chs;
	dialogue* dls;
}story;

void error_handling(char* msg);
void init();
void readmsg(char* msg);
int maketoken(char* msg, char** token);

story* parsedoc(char* docname);
void parseheader(xmlDocPtr doc, xmlNodePtr cur, story* s);
void parsecharacters(xmlDocPtr doc, xmlNodePtr cur, story* s);
void parsescript(xmlDocPtr doc, xmlNodePtr cur, story* s);
void check_connection(char* testip);

socketdata sd;
socketdata speaker_sd[10];
socketdata recv_sd;

char* ip[10];
int ip_count;

int main()
{
	char msg[BUFSIZE];
	char* token[10];

	init();

	while(1)
	{
		// readmsg(msg);

		fgets(msg, BUFSIZE, stdin);
		
		maketoken(msg, token);

		// 스피커모듈 통신 확인
		if(atoi(token[0]) == 1)
			check_connection(token[1]);			
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
	int i;

	/*
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
	*/
	///////////////////////////////

	recv_sd.server_sock = socket(PF_INET, SOCK_DGRAM, 0);
	if(recv_sd.server_sock == -1)
		error_handling("sock() error(udp)");

	memset(&(recv_sd.server_addr), 0 , sizeof(struct sockaddr_in));
	recv_sd.server_addr.sin_family = AF_INET;
	recv_sd.server_addr.sin_addr.s.addr = htonl(INADDR_ANY);
	recv_sd.server_addr.sin_port = htons(10002);

	if(bind(recv_sd.server_sock, (struct sockaddr*)&(recv_sd.server_addr), sizeof(struct sockaddr_in)) == -1)
		error_handling("bind() error");

	///////////////////////////////

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

	ip[0] = "192.168.0.138";
	ip_count = 1;

	for(i=0; i<ip_count; i++)
	{
		speaker_sd[i].server_sock = socket(PF_INET, SOCK_DGRAM, 0);
		if(speaker_sd[i].server_sock == -1)
			error_handling("socket() error(udp)");

		memset(&(speaker_sd[i].server_addr), 0, sizeof(struct sockaddr_in));
		speaker_sd[i].server_addr.sin_family = AF_INET;
		speaker_sd[i].server_addr.sin_addr.s_addr = inet_addr(ip[i]);
		speaker_sd[i].server_addr.sin_port = htons(10001);

		speaker_sd[i].client_addr_size = sizeof(struct sockaddr_in);
	}

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

story* parsedoc(char* docname)
{
	xmlDocPtr doc;
	xmlNodePtr cur;

	xmlChar* content;

	story* s = (story*)malloc(sizeof(story));
	if(s==NULL)
	{
		printf("story space is not created.\n");
		return NULL;
	}

	doc = xmlParseFile(docname);
	if(doc == NULL)
	{
		fprintf(stderr, "Document not parsed successfully.\n");
		return NULL;
	}

	cur = xmlDocGetRootElement(doc);

	if(cur == NULL)
	{
		fprintf(stderr, "empty document.\n");
		xmlFreeDoc(doc);
		return NULL;
	}

	if(xmlStrcmp(cur->name, (const xmlChar*)"format"))
	{
		fprintf(stderr, "document of the wrong type, root node != format");
		xmlFreeDoc(doc);
		return NULL;
	}
		
	cur = cur->xmlChildrenNode;

	while(cur != NULL)
	{
		if(!xmlStrcmp(cur->name, (const xmlChar*)"title"))
		{
			content = xmlNodeGetContent(cur);
			strncpy(s->title, content, sizeof(s->title));
			xmlFree(content);
		}
		else if(!xmlStrcmp(cur->name, (const xmlChar*)"header"))
			parseheader(doc, cur, s);
		else if(!xmlStrcmp(cur->name, (const xmlChar*)"characters"))
			parsecharacters(doc, cur, s);
		else if(!xmlStrcmp(cur->name, (const xmlChar*)"script"))
			parsescript(doc, cur, s);

		cur = cur->next;
	}
	xmlFreeDoc(doc);
	return s;
}

void parseheader(xmlDocPtr doc, xmlNodePtr cur, story* s)
{
	cur = cur->xmlChildrenNode;
	cur = cur->next;
	xmlChar* version = xmlNodeGetContent(cur);
	cur = cur->next->next;
	xmlChar* numofactor = xmlNodeGetContent(cur);
	cur = cur->next->next;
	xmlChar* numofdialogue = xmlNodeGetContent(cur);
	cur = cur->next->next;
	xmlChar* numofline = xmlNodeGetContent(cur);

	s->version = atof(version);
	s->numofactor = atoi(numofactor);
	s->numofdialogue = atoi(numofdialogue);
	s->numofline = atoi(numofline);

	xmlFree(version);
	xmlFree(numofactor);
	xmlFree(numofdialogue);
	xmlFree(numofline);
}

void parsecharacters(xmlDocPtr doc, xmlNodePtr cur, story* s)
{
	xmlChar* name;
	xmlChar* gender;
	xmlChar* age;
	
	int i = 0;

	s->chs = (character*)malloc(sizeof(character)*s->numofactor);
	cur = cur->xmlChildrenNode;

	while(cur!=NULL)
	{
		if(!xmlStrcmp(cur->name, (const xmlChar*)"character"))
		{
			name = xmlGetProp(cur, "name");
			gender = xmlGetProp(cur, "gender");
			age = xmlGetProp(cur, "age");

			strncpy(s->chs[i].name, name, sizeof(s->chs[i].name));
			s->chs[i].age = atoi(age);

			if(!xmlStrcmp(gender, (const xmlChar*)"남자"))
				s->chs[i].gender = 0;
			else
				s->chs[i].gender = 1;
			
			i++;

			xmlFree(name);
			xmlFree(gender);
			xmlFree(age);
		}
		
		cur = cur->next;
	}
}

void parsescript(xmlDocPtr doc, xmlNodePtr cur, story* s)
{
	xmlChar* index;
	xmlChar* actor;
	xmlChar* line;

	int i = 0;

	s->dls = (dialogue*)malloc(sizeof(dialogue)*s->numofline);
	cur = cur->xmlChildrenNode;

	while(cur!=NULL)
	{
		if(!xmlStrcmp(cur->name, (const xmlChar*)"dialogue"))
		{
			xmlNodePtr child = cur->xmlChildrenNode;

			index = xmlGetProp(cur, "index");
			child = child->next;
			actor = xmlNodeGetContent(child);
			child = child->next->next;
			line = xmlNodeGetContent(child);

			s->dls[i].index = atoi(index);
			strncpy(s->dls[i].actor, actor, sizeof(s->dls[i].actor));
			strncpy(s->dls[i].line, line, sizeof(s->dls[i].line));

			xmlFree(index);
			xmlFree(actor);
			xmlFree(line);

			i++;
		}

		cur = cur->next;
	}
}

void check_connection(char* testip)
{
	int i;
	int exist = 0;
	int res;

	char buf[BUFSIZE];

	struct timeval optval = {5, 0};
	int optlen sizeof(optval);

	for(i=0; i<ip_count; i++)
	{
		if(strcmp(testip, ip[i])==0)
		{
			exist = 1;
			break;
		}
	}

	if(exist == 0)
	{
		speaker_sd[i].server_sock = socket(PF_INET, SOCK_DGRAM, 0);
		if(speaker_sd[i].server_sock == -1)
			error_handling("socket() error(speaker_sd)");
		setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &optval, optlen);
	
		memset(&(speaker_sd[i].server_addr), 0, sizeof(struct sockaddr_in));
		speaker_sd[i].server_addr.sin_family = AF_INET;
		speaker_sd[i].server_addr.sin_addr.s_addr = inet_addr(testip);
		speaker_sd[i].server_addr.sin_port = htons(10001);

		speaker_sd[i].client_addr_size = sizeof(struct sockaddr_in);
	}


	sendto(speaker_sd[i].server_sock, "1", strlen("1")+1, 0, (struct sockaddr*)&(speaker_sd[i].server_addr), sizeof(struct sockaddr_in));
	res = recvfrom(speaker_sd[i].server_sock, buf, BUFSIZE, 0, (struct sockaddr*)&(speaker_sd[i].server_addr),&( speaker_sd[i].client_addr_size));
	
	if(res == -1)
	{
		write(sd.server_sock, "true", strlen("true")+1);
		ip_count++;
	}
	else
	{
		write(sd.server_sock, "false", strlen("false")+1);
		close(speaker_sd[i].server_sock);
	}
}
