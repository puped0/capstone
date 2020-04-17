


#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <Python.h>

#define BUFSIZE 1024

typedef struct tagsocketdata
{
	int sock;

	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;

	int client_addr_size;
}socketdata;

typedef struct tagargument_tts
{
	int index;
	char* voice;
	char* line;
}argument_tts;

void error_handling(char* msg);
void init(socketdata* sd);
void readmsg(socketdata* sd, char* msg);
int maketoken(char* msg, char** token);

int createvoice(argument_tts arg);
void* speaking(void* data);

pthread_t speaker_thread;
int speaker_id;

/******************************/
PyThreadState* mainThreadState;
/******************************/

socketdata sd;

int main()
{
	int index = 0;
	init(&sd);

	char msg[BUFSIZE];
	char* token[10];

	/*****************************/
	PyGILState_STATE gilState;
	/*****************************/

	while(1)
	{
		readmsg(&sd, msg);
		
		//////////////////////////
	//	printf("get : ");
	//	fgets(msg, BUFSIZE, stdin);
	//	msg[strlen(msg)-1] = 0;
		//////////////////////////

		maketoken(msg, token);
		
		// 연결 확인 응답
		if(atoi(token[0]) == 1)
		{
			sendto(sd.sock, "0", strlen("0")+1, 0, (struct sockaddr*)&(sd.client_addr), sd.client_addr_size);
		}
		// 대사 받아서 음성파일 생성
		else if(atoi(token[0]) == 2)
		{
			argument_tts arg;
		
			arg.index = index;
			arg.voice = token[1];
			arg.line = token[2];
		
			if(createvoice(arg)==0)
				printf("output%d.wav 생성완료\n", index);
			else
				printf("음성파일 생성 실패\n");

			index++;
		}
		// 이번 차례에 생성된 음성파일 재생
		else if(atoi(token[0]) == 3)
		{
			/******************/
			gilState = PyGILState_Ensure();
			/******************/

			speaker_id = pthread_create(&speaker_thread, NULL, speaking, (void*)index);
			if(speaker_id < 0)
				perror("thread create error : ");
			/***************/
			PyGILState_Release(gilState);
			/***************/	

			index = 0;
		}
		else if(atoi(token[0]) == 100)
			break;
		else
		{
			printf("receive : %s\n", msg);
			sendto(sd.sock, msg, strlen(msg)+1, 0, (struct sockaddr*)&(sd.client_addr), sd.client_addr_size);
		}
	}


	PyEval_RestoreThread(mainThreadState);
	if(Py_FinalizeEx() < 0)
		return 120;
	
	close(sd.sock);
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
	sd->server_addr.sin_port = htons(10001);

	if(bind(sd->sock, (struct sockaddr*)&(sd->server_addr), sizeof(sd->server_addr)) == -1)
		error_handling("bind() error");

	sd->client_addr_size = sizeof(sd->client_addr);

	/////////////////////////////////////////////
	
	setenv("PYTHONPATH", ".", 1);
	
	Py_Initialize();
	
	PyRun_SimpleString("import os");
	PyRun_SimpleString("os.environ[\"GOOGLE_APPLICATION_CREDENTIALS\"]=\"/home/pi/TTS capstone-d09b840abc51.json\"");

	/********************************************/
	PyEval_InitThreads();
	mainThreadState = PyEval_SaveThread();
	/********************************************/
}

void readmsg(socketdata* sd, char* msg)
{
	memset(msg, 0, BUFSIZE);

	recvfrom(sd->sock, msg, BUFSIZE, 0, (struct sockaddr*)&(sd->client_addr), &(sd->client_addr_size));
}

int maketoken(char* msg, char** token)
{
	int i = 0;
	char* ptr = strtok(msg, ",");

	while(ptr != NULL)
	{
		token[i] = ptr;
		ptr = strtok(NULL, ",");
		i++;
	}
	return i;
}

int createvoice(argument_tts arg)
{
	PyObject *pName, *pModule, *pFunc;
	PyObject *pArgs, *pValue;

	/*****************/
	PyGILState_STATE gilState;
	/*****************/

	gilState = PyGILState_Ensure();

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
			
			pValue = PyLong_FromLong(arg.index);
			if(!pValue)
			{
				Py_DECREF(pArgs);
				Py_DECREF(pModule);
				fputs("cannot convert argument\n", stderr);
				return -1;
			}
			PyTuple_SetItem(pArgs, 0, pValue);

			pValue = PyUnicode_FromString(arg.voice);
			if(!pValue)
			{
				Py_DECREF(pArgs);
				Py_DECREF(pModule);
				fputs("cannot convert argument\n", stderr);
				return -1;
			}
			PyTuple_SetItem(pArgs, 1, pValue);
			
			pValue = PyUnicode_FromString(arg.line);
			if(!pValue)
			{
				Py_DECREF(pArgs);
				Py_DECREF(pModule);
				fputs("cannot convert argument\n", stderr);
				return -1;
			}


			PyTuple_SetItem(pArgs, 2, pValue);
		
			pValue = PyObject_CallObject(pFunc, pArgs);
			
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
				return -1;
			}
		}
		else
		{
			if(PyErr_Occurred())
				PyErr_Print();
			fprintf(stdout, "failed to load \"%s\"\n", module_name);
		}
		Py_XDECREF(pFunc);
		Py_DECREF(pModule);
	}
	else
	{
		if(PyErr_Occurred())
			PyErr_Print();
		fprintf(stdout, "failed to load \"%s\"\n", module_name);
	}

	PyGILState_Release(gilState);
	return 0;
}

void* speaking(void* data)
{
	int index = (int)data;

	PyObject *pName, *pModule, *pFunc;
	PyObject *pArgs, *pValue;

	PyGILState_STATE gilState;

	char module_name[20] = "tts";
	char func_name[20] = "line_play";

	/****************************/
	gilState = PyGILState_Ensure();
	/****************************/

	pName = PyUnicode_FromString(module_name);
	pModule = PyImport_Import(pName);
	Py_DECREF(pName);

	if(pModule != NULL)
	{
		pFunc = PyObject_GetAttrString(pModule, func_name);
		if(pFunc && PyCallable_Check(pFunc))
		{
			pArgs = PyTuple_New(1);
			
			pValue = PyLong_FromLong(index);
			if(!pValue)
			{
				Py_DECREF(pArgs);
				Py_DECREF(pModule);
				fputs("cannot convert argument\n", stderr);
				return (void*)-1;
			}
			PyTuple_SetItem(pArgs, 0, pValue);
			
			pValue = PyObject_CallObject(pFunc, pArgs);
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
				return (void*)-1;
			}
		}
		else
		{
			if(PyErr_Occurred())
				PyErr_Print();
			fprintf(stdout, "failed to load \"%s\"\n", module_name);
		}
		Py_XDECREF(pFunc);
		Py_DECREF(pModule);
	}
	else
	{
		if(PyErr_Occurred())
			PyErr_Print();
		fprintf(stdout, "failed to load \"%s\"\n", module_name);
	}

	sendto(sd.sock, "0", strlen("0")+1, 0, (struct sockaddr*)&(sd.client_addr), sd.client_addr_size);

	/****************************/	
	PyGILState_Release(gilState);
	/****************************/

	return (void*)0;
}
