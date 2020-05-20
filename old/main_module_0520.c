


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

	int speaker;
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

int parserole(char* docname, story* s);

void check_connection(char* testip);

void* playstory(void* data);

socketdata sd;
socketdata speaker_sd[10];
socketdata recv_sd;

pthread_mutex_t pause_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t player_thread;
int player_id;


char ip[10][20];
int ip_count;

int ispause;
int isplaying;
int isstop;

int main()
{
	char msg[BUFSIZE];
	char* token[10];
	int i;
	
	int status;

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
			if(isplaying == 0)
			{
				isplaying = 1;
				player_id = pthread_create(&player_thread, NULL, playstory, (void*)token[1]);
				if(player_id < 0)
				{
					perror("thread create error : ");
					exit(0);
				}
			}
		}
		else if(atoi(token[0]) == 3)
		{
			// 대본 일시 정지
			if(ispause == 0)
			{
				pthread_mutex_lock(&pause_mutex);
				ispause = 1;
			}


		}
		else if(atoi(token[0]) == 4)
		{
			// 일시정지 상태에서 재생
			if(ispause == 1)
			{
				pthread_mutex_unlock(&pause_mutex);
				ispause = 0;
			}
		}
		else if(atoi(token[0]) == 5)
		{
			// 대본 완전 정지
			if(isplaying == 1)
			{
				if(ispause == 1)
				{
					pthread_mutex_unlock(&pause_mutex);
					ispause = 0;
				}

				isstop = 1;
				pthread_join(player_thread, (void**)&status);
				isstop = 0;
			}
	
		}
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

	pthread_mutex_destroy(&pause_mutex);

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

	isplaying = 0;
	ispause = 0;
	isstop = 0;

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
			
			if(xmlStrcmp(content, (const xmlChar*)"init"))
			{
				strncpy(ip[ip_count], content, sizeof(ip[0]));
				ip_count++;
			}

			xmlFree(content);
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
	char* title = (char*)data;
	char docname[100];

	strncpy(docname, title, sizeof(docname));
	strcat(docname, ".xml");
	
	story* s = parsedoc(docname);
	if(s == NULL)
		return NULL;

	int numofactor = s->numofactor;
	int numofdialogue = s->numofdialogue;
	int numofline = s->numofline;

	int current_dialogue = 0;
	int current_line = 0;

	int current_line_count = 0;
	int prev_line_count = 0;

	char* line;
	char* voice;
	int speaker_idx;

	char buf[BUFSIZE];
	int i, j;

	if(parserole(title, s) == -1)
	{
		free(s->dls);
		free(s->chs);
		free(s->linecount_per_dialogue);
		free(s);

		return NULL;
	}

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

	/*
	printf("%s\n", s->title);
	printf("%lf, %d, %d, %d\n", s->version, s->numofactor, s->numofdialogue, s->numofline);
	for(int i=0; i<s->numofactor;i++)
		printf("%s, %s, %d, %d, %d\n", s->chs[i].name, s->chs[i].voice, s->chs[i].gender, s->chs[i].age, s->chs[i].speaker);

	for(int i=0; i<s->numofline; i++)
		printf("%d. %s : %s\n", s->dls[i].index, s->chs[s->dls[i].actor].name, s->dls[i].line);

	for(int i=0; i<s->numofdialogue; i++)
		printf("%d ", s->linecount_per_dialogue[i]);
	printf("\n");
	*/

	
	while(current_dialogue == s->dls[current_line].index)
	{
		voice = s->chs[s->dls[current_line].actor].voice;
		line = s->dls[current_line].line;
		speaker_idx = s->chs[s->dls[current_line].actor].speaker;

		current_line++;
		current_line_count++;

		sprintf(buf, "2_ko-KR-Standard-%s_%s", voice, line);
		printf("%d : %s\n",speaker_idx, buf);

		sendto(speaker_sd[speaker_idx].server_sock, buf, strlen(buf)+1, 0,
			(struct sockaddr*)&(speaker_sd[speaker_idx].server_addr), sizeof(struct sockaddr_in));

		usleep(100000);
	}

	// 뜬금없이 recvfrom이 나왔다.
	// 스피커와의 동기화 과정이
	// 스피커의 대사생성  완료 신호 + 스피커의 대사 읽기 완료신호 두 정수신호의 합을 통해
	// 다음 대사를 재생해도 될 지 판단하는데
	// 첫 대사를 보낼 땐 한 타이밍의 딜레이를 발생시키기 위해
	// 대사 하나를 미리 보낸다.
	// 이로 안해 스피커 모듈에서 대사 생성을 완료했다는 신호가 2번 오게 되는데
	// (하나는 바로 위 코드의 신호, 하나는 아래 루프문의 첫 루프 때의 신호)
	// 이 때문에 첫 루프 때 대사 동기화가 맞지 않게 된다.
	// 이를 해결하기 위해선 위의 첫 대사를 보내는 코드의 응답신호를 무시해야 하는데
	// 아래 recvfrom 함수로 그 신호를 받기만 하고 안쓰기만 하면 해결된다.	
	recvfrom(recv_sd.server_sock, buf, BUFSIZE, 0, (struct sockaddr*)&(recv_sd.server_addr), &(recv_sd.client_addr_size));

	printf("--------------\n");
	current_dialogue++;

	prev_line_count = current_line_count;
	current_line_count = 0;

	sleep(3);

	while(1)
	{
		int line_sum = 0;

//		sendto(speaker_sd[0].server_sock, "3", strlen("3")+1, 0, (struct sockaddr*)&(speaker_sd[0].server_addr), sizeof(struct sockaddr_in));
		
		for(j=0; j<ip_count; j++)
			sendto(speaker_sd[j].server_sock, "3", strlen("3")+1, 0, (struct sockaddr*)&(speaker_sd[j].server_addr), sizeof(struct sockaddr_in));

		// pause
		if(ispause == 1)
		{
			pthread_mutex_lock(&pause_mutex);
			pthread_mutex_unlock(&pause_mutex);
		}		

		// stop
		if(isstop == 1)
		{
			while(line_sum != prev_line_count)
			{
				recvfrom(recv_sd.server_sock, buf, BUFSIZE, 0, (struct sockaddr*)&(recv_sd.server_addr), &(recv_sd.client_addr_size));
				line_sum = line_sum + atoi(buf);
			}
			break;
		}

		while(current_dialogue == s->dls[current_line].index && current_dialogue != numofdialogue)
		{
			voice = s->chs[s->dls[current_line].actor].voice;
			line = s->dls[current_line].line;
			speaker_idx = s->chs[s->dls[current_line].actor].speaker;

			current_line++;
			current_line_count++;

			sprintf(buf, "2_ko-KR-Standard-%s_%s", voice, line);
			printf("%d : %s\n",speaker_idx, buf);
			
			// 역할별로 다른 스피커에 전송할 수도 있음... 추후 구현
			sendto(speaker_sd[speaker_idx].server_sock, buf, strlen(buf)+1, 0, (struct sockaddr*)&(speaker_sd[speaker_idx].server_addr), sizeof(struct sockaddr_in));
			
			usleep(100000);	
		}
		
		printf("------------\n");	
		current_dialogue++;

		while(line_sum != prev_line_count + current_line_count)
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

	isplaying = 0;
}

int parserole(char* docname, story* s)
{
	xmlDocPtr doc;
	xmlNodePtr tag_script;
	
	xmlNodePtr act;
	xmlNodePtr userchoice;

	xmlChar* title;

	xmlChar* actor;
	xmlChar* index;

	int numofactor = s->numofactor;
	int i;


	doc = xmlParseFile("role.xml");
	if(doc == NULL)
	{
		fprintf(stderr, "Document not parsed successfully.\n");
		return -1;
	}

	tag_script = xmlDocGetRootElement(doc);

	if(tag_script == NULL)
	{
		fprintf(stderr, "empty document.\n");
		xmlFreeDoc(doc);
		return -1;
	}

	if(xmlStrcmp(tag_script->name, (const xmlChar*)"scriptList"))
	{
		fprintf(stderr, "document of the wrong type, root node != scriptList");

		xmlFree(doc);
		return -1;
	}

	tag_script = tag_script->xmlChildrenNode;
	

	while(tag_script != NULL)
	{
		if(!xmlStrcmp(tag_script->name, (const xmlChar*)"script"))
		{
			act = tag_script->xmlChildrenNode->next;
			title = xmlNodeGetContent(act);

			if(!xmlStrcmp(title, (const xmlChar*)docname))
			{
				
				act = act->next->next;
				userchoice = act->next->next->xmlChildrenNode->next->next->next;
				act = act->xmlChildrenNode->next;

				while(act != NULL || userchoice != NULL)
				{
					if(!xmlStrcmp(act->name, (const xmlChar*)"char") && !xmlStrcmp(userchoice->name, (const xmlChar*)"index"))
					{
						actor = xmlNodeGetContent(act);
						index = xmlNodeGetContent(userchoice);
					
						for(i=0; i<numofactor; i++)
						{
							if(!xmlStrcmp(actor, (const xmlChar*)(s->chs[i].name)))
							{
								printf("%s\n", index);
								s->chs[i].speaker = atoi(index);
								break;
							}
						}
						
						free(actor);
						free(index);

					}

					act = act->next;
					userchoice = userchoice->next;

				}
				
				xmlFree(title);
				break;
			}

			xmlFree(title);
		}

		tag_script = tag_script->next;
	}

	if(tag_script == NULL)
	{
		fprintf(stderr, "cannot find role script : %s\n", docname);
		xmlFreeDoc(doc);
		return -1;
	}
	else
	{
		xmlFreeDoc(doc);
		return 0;
	}
}
