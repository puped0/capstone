

#define PY_SSIZE_T_CLEAN

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <Python.h>

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
	
int init();
story* parsedoc(char* docname);
void parseheader(xmlDocPtr doc, xmlNodePtr cur, story* s);
void parsecharacters(xmlDocPtr doc, xmlNodePtr cur, story* s);
void parsescript(xmlDocPtr doc, xmlNodePtr cur, story* s);
int calltts(char*);

int main(int argc, char** argv)
{
	int result;
	char* docname = "kong.xml";
	story* s;
	s = parsedoc(docname);

	
	printf("%s\n", s->title);
	printf("%lf, %d, %d, %d\n", s->version, s->numofactor, s->numofdialogue, s->numofline);
	for(int i=0; i<s->numofactor;i++)
		printf("%s, %d, %d\n", s->chs[i].name, s->chs[i].gender, s->chs[i].age);
	for(int i=0; i<s->numofline;i++)
		printf("%d. %s : %s\n", s->dls[i].index, s->dls[i].actor, s->dls[i].line);

	
	init();

	/*
	for(int i=0; i<s->numofline; i++)
	{
		calltts(s->dls[i].line);
		printf("%d.%s : %s\n", s->dls[i].index, s->dls[i].actor, s->dls[i].line);
	}

	free(s->dls);
	free(s->chs);
	free(s);
*/
	if(Py_FinalizeEx()<0)
		return 120;

	return 0;
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

int calltts(char* line)
{
	PyObject *pName, *pModule, *pFunc;
	PyObject *pArgs, *pValue;

	char module_name[20] = "tts";
	char func_name[20] = "tts_func";

//	setenv("PYTHONPATH", ".", 1);

//	Py_Initialize();

//	PyRun_SimpleString("import os");
//	PyRun_SimpleString("os.environ[\"GOOGLE_APPLICATION_CREDENTIALS\"]=\"/home/pi/TTS capstone-d09b840abc51.json\"");

	pName = PyUnicode_FromString(module_name);
	pModule = PyImport_Import(pName);
	Py_DECREF(pName);

	if(pModule != NULL)
	{
		pFunc = PyObject_GetAttrString(pModule, func_name);
		if(pFunc && PyCallable_Check(pFunc))
		{
			pArgs = PyTuple_New(1);
			pValue = PyUnicode_FromString(line);
			if(!pValue)
			{
				Py_DECREF(pArgs);
				Py_DECREF(pModule);
				fprintf(stderr, "Cannot convert argument\n");
				return 1;
			}

			PyTuple_SetItem(pArgs, 0, pValue);

			pValue = PyObject_CallObject(pFunc, pArgs);
			
			if(pValue != NULL)
			{
				printf("Result of call : %ld(from python module)\n", PyLong_AsLong(pValue));
				Py_DECREF(pValue);
			}
			else
			{
				Py_DECREF(pName);
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

//	if(Py_FinalizeEx()<0)
//		return 120;

	return 0;
}

int init()
{
	setenv("PYTHONPATH", ".", 1);

	Py_Initialize();
	
	PyRun_SimpleString("import os");
	PyRun_SimpleString("os.environ[\"GOOGLE_APPLICATION_CREDENTIALS\"]=\"/home/pi/TTS capstone-d09b840abc51.json\"");
}

