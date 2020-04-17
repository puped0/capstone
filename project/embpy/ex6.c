

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <Python.h>


typedef struct tagargument
{
	int index;
	char* voice;
	char* line;
}argument;

int init();
int calltts(argument*);

int main()
{
	argument arg;
	arg.index = 10;
	arg.voice = "male";
	arg.line = "hello world";	

	init();
	calltts(&arg);

	if(Py_FinalizeEx()<0)
		return 120;

	return 0;
}

int init()
{
	setenv("PYTHONPATH", ".", 1);
	Py_Initialize();

	PyRun_SimpleString("import os");
	PyRun_SimpleString("os.environ[\"GOOGLE_APPLICATION_CREDENTIALS\"]=\"/home/pi/TTS capstone-d09b840abc51.json\"");
}

int calltts(argument* arg)
{
	PyObject *pName, *pModule, *pFunc;
	PyObject *pArgs, *pValue;

	char* line = "hello world";

	char module_name[100] = "funcs";
	char func_name[20] = "tts_ex";

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
			
			pValue = PyObject_CallObject(pFunc, pArgs);

			if(pValue != NULL)
			{
				printf("result of call : %ld(from python module)\n", PyLong_AsLong(pValue));
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
			if(PyErr_Occurred())
				PyErr_Print();
			fprintf(stderr, "cannot find function \"%s\"\n", func_name);
		}

		Py_XDECREF(pFunc);
		Py_DECREF(pModule);
	}
	else
	{	
		if(PyErr_Occurred())
			PyErr_Print();
		fprintf(stderr, "failed to load \"%s\"\n", module_name);
		return 1;
	}
	
	return 0;
}

