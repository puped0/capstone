


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
	char voice[20];
	char line[500];
}argument_tts;

void error_handling(char* msg);
void init(socketdata* sd);
void readmsg(socketdata* sd, char* msg);
int maketoken(char* msg, char** token);

void* createvoice(void* data);
void* speaking(void* data);

void stopline();
void pauseline();
void unpauseline();

pthread_t wav_create_thread[10];
int wav_create_id[10];

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

	argument_tts* arg;

	/*****************************/
	PyGILState_STATE gilState;
	/*****************************/

	while(1)
	{
		readmsg(&sd, msg);
		printf("%d : %s\n", index, msg);	
		//////////////////////////
	//	printf("get : ");
	//	fgets(msg, BUFSIZE, stdin);
	//	msg[strlen(msg)-1] = 0;
		//////////////////////////

		maketoken(msg, token);

		// 연결 확인 응답
		if(atoi(token[0]) == 1)
		{
			printf("connection check\n");
			sendto(sd.sock, "0", strlen("0")+1, 0, (struct sockaddr*)&(sd.client_addr), sd.client_addr_size);
		}
		// 대사 받아서 음성파일 생성
		else if(atoi(token[0]) == 2)
		{
			arg = (argument_tts*)malloc(sizeof(argument_tts));

			arg->index = index;
			strcpy(arg->voice, token[1]);
			strcpy(arg->line, token[2]);

			/******************/
			gilState = PyGILState_Ensure();
			/******************/
		
			wav_create_id[index] = pthread_create(&wav_create_thread[index], NULL, createvoice, (void*)arg);
			if(wav_create_id[index] < 0)
				perror("thread create error : ");
			
			/***************/
			PyGILState_Release(gilState);
			/***************/	

			/*
			if(createvoice(arg)==0)
				printf("output%d.wav 생성완료\n", index);
			else
				printf("음성파일 생성 실패\n");
			*/

			index++;
		}
		// 이번 차례에 생성된 음성파일 재생
		else if(atoi(token[0]) == 3)
		{
			if(index != 0)
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
		}
		else if(atoi(token[0]) == 4)
		{
			// 대본이 정지됨
			// 대본이 정지되어 index를 0으로 초기화
			// 뜬금없이 대사가 나오는 것을 방지
			
			/******************/
			gilState = PyGILState_Ensure();
			/******************/

			stopline();
	
			/***************/
			PyGILState_Release(gilState);
			/***************/	
			
			index = 0;
		}
		else if(atoi(token[0]) == 5)
		{
			// 대사 일시정지
			/******************/
			gilState = PyGILState_Ensure();
			/******************/
	
			pauseline();
	
			/***************/
			PyGILState_Release(gilState);
			/***************/	
	
		}
		else if(atoi(token[0]) == 6)
		{
			// 대사 다시재생
			/******************/
			gilState = PyGILState_Ensure();
			/******************/
	 
			unpauseline();
	
			/***************/
			PyGILState_Release(gilState);
			/***************/	
		}
		else if(atoi(token[0]) == 100)
			break;
		else // 없어도 됨 디버깅용
		{
			printf("receive : %s\n", msg);
			
			sd.client_addr.sin_port = htons(10002);
			sendto(sd.sock, msg, strlen(msg)+1, 0, (struct sockaddr*)&(sd.client_addr), sd.client_addr_size);
		}
	}


	PyEval_RestoreThread(mainThreadState);

	PyRun_SimpleString("pygame.mixer.quit()");
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
	PyRun_SimpleString("import pygame");
	PyRun_SimpleString("import time");
	PyRun_SimpleString("pygame.mixer.init(24000, -16, 1, 2048)");

	/********************************************/
	Py_DECREF(PyImport_ImportModule("threading"));
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
	char* ptr = strtok(msg, "_");

	while(ptr != NULL)
	{
		token[i] = ptr;
		ptr = strtok(NULL, "_");
		i++;
	}
	return i;
}

void* createvoice(void* data)
{
	argument_tts* arg;

	PyObject *pName, *pModule, *pFunc;
	PyObject *pArgs, *pValue;

	/*****************/
	PyGILState_STATE gilState;
	/*****************/

	/********************************/
	gilState = PyGILState_Ensure();
	/********************************/

	char module_name[20] = "tts";
	char func_name[20] = "tts_func";

	arg = (argument_tts*)data;

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
				return (void*)-1;
			}
			PyTuple_SetItem(pArgs, 0, pValue);

			pValue = PyUnicode_FromString(arg->voice);
			if(!pValue)
			{
				Py_DECREF(pArgs);
				Py_DECREF(pModule);
				fputs("cannot convert argument\n", stderr);
				return (void*)-1;
			}
			PyTuple_SetItem(pArgs, 1, pValue);
			
				pValue = PyUnicode_FromString(arg->line);
			if(!pValue)
			{
				Py_DECREF(pArgs);
				Py_DECREF(pModule);
				fputs("cannot convert argument\n", stderr);
				return (void*)-1;
			}


			PyTuple_SetItem(pArgs, 2, pValue);
		
			pValue = PyObject_CallObject(pFunc, pArgs);
			
			if(pValue != NULL)
			{
			//	printf("Result of call : %ld(from python module)\n");
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
	
	printf("output%d.wav 생성완료\n", arg->index);

	free(data);

	sd.client_addr.sin_port = htons(10002);
	sendto(sd.sock, "1", strlen("1")+1, 0, (struct sockaddr*)&(sd.client_addr), sd.client_addr_size);

	/*****************************/
	PyGILState_Release(gilState);
	/*****************************/

	return (void*)0;
}

void* speaking(void* data)
{
	int index = (int)data;

	PyObject *pName, *pModule, *pFunc;
	PyObject *pArgs, *pValue;

	PyGILState_STATE gilState;

	/****************************/
	gilState = PyGILState_Ensure();
	/****************************/

	char module_name[20] = "tts";
	char func_name[20] = "line_play";

	char msg[10];

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
			//	printf("Result of call : %ld(from python module)\n");
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

	sprintf(msg, "%d", index);
	
	sd.client_addr.sin_port = htons(10002);
	sendto(sd.sock, msg, strlen(msg)+1, 0, (struct sockaddr*)&(sd.client_addr), sd.client_addr_size);

	/****************************/	
	PyGILState_Release(gilState);
	/****************************/

	return (void*)0;
}

void stopline()
{
	PyRun_SimpleString("pygame.mixer.stop()");
}

void pauseline()
{
	PyRun_SimpleString("pygame.mixer.pause()");
}

void unpauseline()
{
	PyRun_SimpleString("pygame.mixer.unpause()");
}
