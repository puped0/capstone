

#define PY_SSIZE_T_CLEAN

#include <Python.h>
#include <stdio.h>
#include <stdlib.h>

int call_func(char*, char*);

int main(int argc, char** argv[])
{
	char module_name[20] = "tts";
	char func_name[20] = "tts_func";

	int result = call_func(module_name, func_name);

	return result;
}

int call_func(char* module_name, char* func_name)
{
	PyObject *pName, *pModule, *pFunc;
	PyObject *pArgs, *pValue;


	setenv("PYTHONPATH", ".", 1);

	Py_Initialize();

	PyRun_SimpleString("import os");
	PyRun_SimpleString("os.environ[\"GOOGLE_APPLICATION_CREDENTIALS\"]=\"/home/pi/TTS capstone-d09b840abc51.json\"");

	pName = PyUnicode_FromString(module_name);
	pModule = PyImport_Import(pName);
	Py_DECREF(pName);
	if(pModule != NULL)
	{
		pFunc = PyObject_GetAttrString(pModule, func_name);
		if(pFunc && PyCallable_Check(pFunc))
		{
			pValue = PyObject_CallObject(pFunc, NULL);
			if(pValue != NULL)
			{
				printf("Result of call : %ld\n", PyLong_AsLong(pValue));
				Py_DECREF(pValue);
			}
			else
			{
				Py_DECREF(pFunc);
				Py_DECREF(pModule);
				PyErr_Print();
				fprintf(stderr, "Call failed\n");
				return 1;
			}

		}
		else
		{
			if(PyErr_Occurred())
				PyErr_Print();
			fprintf(stderr, "Cannot find function \"%s\"\n", func_name);
		}
		Py_XDECREF(pFunc);
		Py_DECREF(pModule);
	}
	else
	{
		if(PyErr_Occurred())
			PyErr_Print();
		fprintf(stderr, "Failed to load \"%s\"\n", module_name);
		return 1;
	}

	if(Py_FinalizeEx()<0)
		return 120;

	return 0;
}
