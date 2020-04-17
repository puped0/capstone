



#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <Python.h>1

#define BUFSIZE 1024

typedef struct tagsocketdata
{
	int sock;

	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;

	int client_addr_size;
}socketdata;

typedef struct tagargument
{
	int index;
	char* voice;
	char* line;
}argument;

void error_handling(char* msg);
void init(socketdata* sd);
void readmsg(socketdata* sd, char* msg);
int maketoken(char* msg, char** token);

void* speaking(void* data);

pthread_t speaker_thread[10];
int thread_state[10];

int main()
{
	socketdata sd;
	init(&sd);

	char msg[BUFSIZE];
	char* token[10];

	while(1)
	{
	//	readmsg(&sd, msg);
		fgets(msg, BUFZISE, stdin);

		maketoken(msg, token);
		
		// 연결 확인 응답
		if(atoi(token[0]) == 1)
		{
			sendto(sd.sock, "0", strlen("0")+1, 0, (struct sockaddr*)&(sd.client_addr), sd.client_addr_size);
		}
		// 대사 받아서 쓰레드 생성한 뒤 재생
		else if(atoi(token[0]) == 2)
		{
			argument th_arg;
			int index;

			while(1)
			{
				if(thread_state[index] == 0)
					break;
				index++;
			}

			th_arg.index = index;
			th_arg.voice = token[1];
			th_arg.line = token[2];
			
			thread_state[index] = pthread_create(&(speaker_thread[index]), NULL, speaking, (void*)&th_arg);
			if(thread_state[index]<0)
			{
				perror("thread create error : ");
			}
		}
		else if(atoi(token[0]) == 3){}
		else if(atoi(token[0]) == 4){}
		else
		{
			printf("receive : %s\n", msg);
			sendto(sd.sock, msg, strlen(msg)+1, 0, (struct sockaddr*)&(sd.client_addr), sd.client_addr_size);
		}
	}

	return 0;
}

void error_handling(char* msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

void init(socketdata* sd)
{
	sd->sock = socket(PF_INET, SOCK_DGRAM, 0);

	if(sd->sock == -1)
		error_handling("socket() error");

	memset(&(sd->server_addr), 0, sizeof(sd->server_addr));
	sd->server_addr.sin_family = AF_INET;
	sd->server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	sd->server_addr.sin_port = htons(10000);

	if(bind(sd->sock, (struct sockaddr*)&(sd->server_addr), sizeof(sd->server_addr)) == -1)
		error_handling("bind() error");

	sd->client_addr_size = sizeof(sd->client_addr);

	/////////////////////////////////////////////
	
	memset(speaker_thread, 0, sizeof(speaker_thread));
	memset(thread_state, 0, sizeof(thread_state));

	/////////////////////////////////////////////
	
	setenv("PYTHONPATH", ".", 1);
	
	Py_Initialize();

	PyRun_SimpleString("import.os");
	PyRun_SimpleString("os.environ[\"GOOGLE_APPLICATION_CREDENTIALS\"]=\"/home/pi/TTS capstone-d09b840abc51.json\"");

}

void readmsg(socketdata* sd, char* msg)
{
	memset(msg, 0, BUFSIZE);

	recvfrom(sd->sock, msg, BUFSIZE, 0, (struct sockaddr*)&(sd->client_addr), &(sd->client_addr_size));
}

int maketoken(char* msg, char** token)
{
	int i = 0;
	char* ptr = strtok(msg, "_");

	while(ptr != NULL)
	{
		token[i] = ptr;
		ptr = strtok(NULL, "_");
		i++;
	}
	return i;
}

void* speaking(void* data)
{
	argument* arg = (argument*)data;

	PyObject *pName, *pModule, *pFunc;
	PyObject *pArgs, *pValue;

	char module_name[20] = "tts";
	char func_name[20] = "tts_func";

	pName = PyUnicode_FromString(module_name);
	pModule = PyImport_Import(pName);
	Py_DECREF(pName);

	if(pModule != NULL)
	{
		pFunc = PyObject_GetAttrString(pModule, func_name);
		if(pFunc && PyCallable_Check(pFunc))
		{
			pArgs = PyTuple_New(3);
		

			pValue = PyLong_FromLong(arg->index);
			if(!pValue)
			{
				Py_DECREF(pArgs);
				Py_DECREF(pModule);
				fputs("cannot convert argument\n", stderr);
				return 1;
			}
			PyTuple_SetItem(pArgs, 0, pValue);


			pValue = PyUnicode_FromString(arg->voice);
			if(!pValue)
			{
				Py_DECREF(pArgs);
				Py_DECREF(pModule);
				fputs("cannot convert argument\n", stderr);
				return 1;
			}
			PyTuple_SetItem(pArgs, 1, pValue);

			pValue = PyUnicode_FromString(arg->line);
			if(!pValue)
			{
				Py_DECREF(pArgs);
				Py_DECREF(pModule);
				fputs("cannot convert argument\n", stderr);
				return 1;
			}


			PyTuple_SetItem(pArgs, 2, pValue);

			pValue = PyObject_CallObject(pfunc, pArgs);

			if(pValue != NULL)
			{
				printf("Result of call : %ld(from python module)\n");
				Py_DECREF(pValue);
			}
			else
			{
				Py_DECREF(pName);
				Py_DECREF(pModule);
				PyErr_Print();
				fputs("call failed\n", stderr);
				return 1;
			}
		}
		else
		{
			if(PyErr_Occured())
				PyErr_Print();
			fputs("failed to load \"%s\"\n", module_name);
		}
		Py_XDECREF(pFunc);
		Py_DECREF(pModule);
	}
	else
	{
		if(PyErr_Occured())
			PyErr_Print();
		fputs("failed to load \"%s\"\n", module_name);
	}

	// if(Py_FinalizeEx()<0)
	//	return 120;
	
	thread_state[index] = 0;

	return 0;
}
