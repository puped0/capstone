

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

typedef struct tagcharacter
{
	char name[20];
	int gender;
	int age;
}character;

typedef struct tagdaialogue
{
	char actor[20];
	char line[500];
}daialogue;

typedef struct tagstory
{
	char title[30];

	double version;
	int numofactor;
	int numofdialogue;
	int numofline;

	character* chs;
	daialogue* dls;
}story;
	


story* parsedoc(char* docname);
void parseheader(xmlDocPtr doc, xmlNodePtr cur, story* s);
void parsecharacters(xmlDocPtr doc, xmlNodePtr cur, story* s);
void parsescript(xmlDocPtr doc, xmlNodePtr cur, story* s);

int main(int argc, char** argv)
{
	char* docname = "story.xml";
	story* s;
	s = parsedoc(docname);

	printf("%s\n", s->title);
	printf("%lf, %d, %d, %d\n", s->version, s->numofactor, s->numofdialogue, s->numofline);
	for(int i=0; i<s->numofactor;i++)
		printf("%s, %d, %d\n", s->chs[i].name, s->chs[i].gender, s->chs[i].age);

	free(s->chs);
	free(s);

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
		else if(!xmlStrcmp(cur->name, (const xmlChar*)"script")){}
//			parsescript(doc, cur, s);

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
	s->numofline = atoi(nunofline);

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

}
