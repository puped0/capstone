


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
	char* voice;
	int gender;
	int age;
}character;

typedef struct tagdialogue
{
	int index;
	int actor;
	char line[500];
}dialogue;

typedef struct tagstory
{
	char title[30];

	double version;
	int numofactor;
	int numofdialogue;
	int numofline;

	int* linecount_per_dialogue;

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

void* playstory(void* data);

socketdata sd;
socketdata speaker_sd[10];
socketdata recv_sd;

pthread_t player_thread;
int player_id;

char ip[10][20];
int ip_count;

int main()
{
	char msg[BUFSIZE];
	char* token[10];
	int i;

	init();

	while(1)
	{
		// readmsg(msg);

		////////////////////////////
		fgets(msg, BUFSIZE, stdin);
		msg[strlen(msg)-1] = 0;		
		////////////////////////////

		maketoken(msg, token);

		// 스피커모듈 통신 확인
		if(atoi(token[0]) == 1)
			check_connection(token[1]);			
		// 대본 시작
		else if(atoi(token[0]) == 2)
		{
			player_id = pthread_create(&player_thread, NULL, playstory, (void*)token[1]);
			if(player_id < 0)
			{
				perror("thread create error : ");
				exit(0);
			}
		}
		// 정지, 재생, 앞으로, 뒤로 등등...
		else if(atoi(token[0]) == 3){}
		else if(atoi(token[0]) == 4){}
		else if(atoi(token[0]) == 100)
		{
			for(i=0; i<ip_count; i++)
				sendto(speaker_sd[i].server_sock, "100", strlen("100")+1, 0, 
						(struct sockaddr*)&(speaker_sd[i].server_addr), sizeof(struct sockaddr_in));
			break;
		
		}
		else{}

	}

	for(i=0; i<ip_count; i++)
		close(speaker_sd[i].server_sock);
	close(recv_sd.server_sock);
	close(sd.server_sock);
	close(sd.client_sock);

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

	struct timeval optval = {5, 0};
	int optlen = sizeof(optval);

	xmlDocPtr ipdoc;
	xmlNodePtr cur;
	xmlChar* content;

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
	recv_sd.server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
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

	ip_count = 0;

	ipdoc = xmlParseFile("ip.xml");
	if(ipdoc == NULL)
	{
		fprintf(stderr, "Document not parsed successfully.\n");
		exit(1);
	}

	cur = xmlDocGetRootElement(ipdoc);
	if(cur == NULL)
	{
		fprintf(stderr, "empty document.\n");
		xmlFreeDoc(ipdoc);
		exit(1);
	}

	cur = cur->xmlChildrenNode;

	while(cur != NULL)
	{
		if(!xmlStrcmp(cur->name, (const xmlChar*)"ip"))
		{
			content = xmlNodeGetContent(cur);
			strncpy(ip[ip_count], content, sizeof(ip[0]));
			xmlFree(content);

			ip_count++;
		}

		cur = cur->next;
	}

	for(i=0; i<ip_count; i++)
	{
		printf("%s\n", ip[i]);
		speaker_sd[i].server_sock = socket(PF_INET, SOCK_DGRAM, 0);
		if(speaker_sd[i].server_sock == -1)
			error_handling("socket() error(udp)");
		setsockopt(speaker_sd[i].server_sock, SOL_SOCKET, SO_RCVTIMEO, &optval, optlen);

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
	char* ptr = strtok(msg, "_");
	
	while(ptr != NULL)
	{
		token[i] = ptr;
		ptr = strtok(NULL, "_");
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
	int j = 0;
	int linecount = 0;
	int cur_index = 0;

	s->dls = (dialogue*)malloc(sizeof(dialogue)*s->numofline);
	s->linecount_per_dialogue = (int*)malloc(sizeof(int)*s->numofdialogue);
	
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
			strncpy(s->dls[i].line, line, sizeof(s->dls[i].line));

			for(j=0; j<s->numofactor; j++)
			{
				if(!xmlStrcmp(actor, (const xmlChar*)(s->chs[j].name)))
				{
					s->dls[i].actor = j;
					break;
				}
			}
			
			if(cur_index != s->dls[i].index)
			{
				s->linecount_per_dialogue[cur_index] = linecount;
				linecount = 1;
				cur_index++;
			}
			else
				linecount++;

			xmlFree(index);
			xmlFree(actor);
			xmlFree(line);

			i++;
		}

		cur = cur->next;
	}

	s->linecount_per_dialogue[cur_index] = linecount;
}

void check_connection(char* testip)
{
	int i;
	int exist = 0;
	int res;

	char buf[BUFSIZE];

	struct timeval optval = {5, 0};
	int optlen = sizeof(optval);

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
		setsockopt(speaker_sd[i].server_sock, SOL_SOCKET, SO_RCVTIMEO, &optval, optlen);
	
		memset(&(speaker_sd[i].server_addr), 0, sizeof(struct sockaddr_in));
		speaker_sd[i].server_addr.sin_family = AF_INET;
		speaker_sd[i].server_addr.sin_addr.s_addr = inet_addr(testip);
		speaker_sd[i].server_addr.sin_port = htons(10001);

		speaker_sd[i].client_addr_size = sizeof(struct sockaddr_in);
	}


	sendto(speaker_sd[i].server_sock, "1", strlen("1")+1, 0, (struct sockaddr*)&(speaker_sd[i].server_addr), sizeof(struct sockaddr_in));
	res = recvfrom(speaker_sd[i].server_sock, buf, BUFSIZE, 0, (struct sockaddr*)&(speaker_sd[i].server_addr),&( speaker_sd[i].client_addr_size));

	if(res != -1)
	{
//		write(sd.server_sock, "true", strlen("true")+1);
		
		if(exist == 0)
		{
			ip_count++;
			strncpy(ip[i], testip, sizeof(ip[0]));
		}

		printf("connection checked : %s\n", testip);
	}
	else
	{
//		write(sd.server_sock, "false", strlen("false")+1);
		
		if(exist == 0)		
			close(speaker_sd[i].server_sock);

		printf("not connected : %s\n", testip);
	}
}



void* playstory(void* data)
{
	char* docname = (char*)data;
	story* s = parsedoc(docname);

	int numofactor = s->numofactor;
	int numofdialogue = s->numofdialogue;
	int numofline = s->numofline;

	int current_dialogue = 0;
	int current_line = 0;

	int current_line_count = 0;
	int prev_line_count = 0;

	char* line;
	char* voice;

	char buf[BUFSIZE];
	int i;

	for(i=0; i<numofactor; i++)
	{
		if(s->chs[i].gender == 1)
		{
			if(s->chs[i].age < 20)
				s->chs[i].voice = "A";
			else
				s->chs[i].voice = "B";
		}
		else
		{
			if(s->chs[i].age <20)
				s->chs[i].voice = "C";
			else
				s->chs[i].voice = "D";

		}
	}

	while(current_dialogue == s->dls[current_line].index)
	{
		voice = s->chs[s->dls[current_line].actor].voice;
		line = s->dls[current_line].line;
		
		current_line++;
		current_line_count++;

		sprintf(buf, "2_ko-KR-Standard-%s_%s", voice, line);
		printf("%s\n", buf);

		sendto(speaker_sd[0].server_sock, buf, strlen(buf)+1, 0,
			(struct sockaddr*)&(speaker_sd[0].server_addr), sizeof(struct sockaddr_in));

		usleep(100000);
	}

	printf("--------------\n");
	current_dialogue++;

	prev_line_count = current_line_count;
	current_line_count = 0;

	sleep(5);

	while(1)
	{
		int line_sum = 0;

		sendto(speaker_sd[0].server_sock, "3", strlen("3")+1, 0, (struct sockaddr*)&(speaker_sd[0].server_addr), sizeof(struct sockaddr_in));


		while(current_dialogue == s->dls[current_line].index && current_dialogue != numofdialogue)
		{
			voice = s->chs[s->dls[current_line].actor].voice;
			line = s->dls[current_line].line;

			current_line++;
			current_line_count++;

			sprintf(buf, "2_ko-KR-Standard-%s_%s", voice, line);
			printf("%s\n", buf);
			
			// 역할별로 다른 스피커에 전송할 수도 있음... 추후 구현
			sendto(speaker_sd[0].server_sock, buf, strlen(buf)+1, 0, (struct sockaddr*)&(speaker_sd[0].server_addr), sizeof(struct sockaddr_in));
			
			usleep(100000);	
		}
		
		printf("------------\n");	
		current_dialogue++;

		while(line_sum != prev_line_count)
		{
			recvfrom(recv_sd.server_sock, buf, BUFSIZE, 0, (struct sockaddr*)&(recv_sd.server_addr), &(recv_sd.client_addr_size));
			line_sum = line_sum + atoi(buf);
		}

		prev_line_count = current_line_count;
		current_line_count = 0;

		if(prev_line_count == 0)
			break;
	}



	printf("end story\n");

	free(s->dls);
	free(s->chs);
	free(s->linecount_per_dialogue);
	free(s);
}



/*
void* playstory(void* data)
{
	char* docname = (char*)data;
	story* s = parsedoc(docname);

	int numofactor = s->numofactor;
	int numofdialogue = s->numofdialogue;
	int numofline = s->numofline;

	int index = 0;
	int current_line = 0;

	int i;

	char* voice;
	char* line;
	int cur_line_count = 0;
	int prev_line_count = 0;

	char buf[BUFSIZE];

	for(i=0; i<numofactor; i++)
	{
		if(s->chs[i].gender == 1)
		{
			if(s->chs[i].age < 20)
				s->chs[i].voice = "A";
			else
				s->chs[i].voice = "B";
		}
		else
		{
			if(s->chs[i].age <20)
				s->chs[i].voice = "C";
			else
				s->chs[i].voice = "D";

		}
	}
*/

	/*
	while(1)
	{
		if(current_line!=0)
		{
			sendto(speaker_sd[0].server_sock, "3", strlen("3")+1, 0,
				(struct sockaddr*)&(speaker_sd[0].server_addr), sizeof(struct sockaddr_in));
		}
		
		if(current_line != numofline)
		{
			while(1)
			{
				if(index == s->dls[current_line].index)
				{
					for(i=0; i<numofactor; i++)
					{
						if(strcmp(s->dls[current_line].actor, s->chs[i].name) == 0)
						{
							voice = s->chs[i].voice;
							break;
						}
					}

					line = s->dls[current_line].line;
					current_line++;
					cur_line_count++;
					
					sprintf(buf, "2_ko-KR-Standard-%s_%s", voice, line);
					printf("%s\n", buf);
					sendto(speaker_sd[0].server_sock, buf, strlen(buf)+1, 0,
							(struct sockaddr*)&(speaker_sd[0].server_addr), sizeof(struct sockaddr_in));
				
					usleep(100000);
				}
				else
				{
					index++;
					break;
				}
			}
		}

		printf("----------------------\n");

		if(current_line != 0)
		{
			int line_sum = 0;
			while(line_sum != prev_line_count)
			{
				recvfrom(recv_sd.server_sock, buf, BUFSIZE, 0,
						(struct sockaddr*)&(recv_sd.server_addr), &(recv_sd.client_addr_size));
				line_sum += atoi(buf);
			}


			prev_line_count = cur_line_count;
			cur_line_count = 0;
		}
		else
			sleep(5);

		if(current_line == numofline)
			break;
	}
	*/

	/*	
	printf("%s\n", s->title);
	printf("%lf, %d, %d, %d\n", s->version, s->numofactor, s->numofdialogue, s->numofline);
	for(int i=0; i<s->numofactor;i++)
		printf("%s, %d, %d\n", s->chs[i].name, s->chs[i].gender, s->chs[i].age);
	
	for(int i=0; i<s->numofline; i++)
		printf("%d. %s : %s\n", s->dls[i].index, s->chs[s->dls[i].actor].name, s->dls[i].line);

	for(int i=0; i<s->numofdialogue; i++)
		printf("%d ", s->linecount_per_dialogue[i]);
	printf("\n");
	*/
	/*
	//for(int i=0; i<s->numofline;i++)
	for(int i=0; i<3; i++)
	{
		sprintf(buf, "2_ko-KR-Standard-%s_%s", "A", s->dls[i].line);
		
		sendto(speaker_sd[0].server_sock, buf, strlen(buf)+1, 0,
			(struct sockaddr*)&(speaker_sd[0].server_addr), sizeof(struct sockaddr_in));
		
		sleep(1);
		
		printf("%s\n", buf);

		sendto(speaker_sd[0].server_sock, "3", strlen("3")+1, 0,
			(struct sockaddr*)&(speaker_sd[0].server_addr), sizeof(struct sockaddr_in));

		recvfrom(recv_sd.server_sock, buf, BUFSIZE, 0,
			(struct sockaddr*)&(recv_sd.server_addr), &(recv_sd.client_addr_size));
	}
	*/		
/*
	printf("end story\n");

	free(s->dls);
	free(s->chs);
	free(s->linecount_per_dialogue);
	free(s);
}

*/
