
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

static void parsedoc(char* docname);
static void parseinfo(xmlDocPtr doc, xmlNodePtr cur);

int main(int argc, char** argv)
{
	char* docname;
	if(argc <= 1)
	{
		printf("usage : %s docname\n", argv[0]);
		return 0;
	}

	docname = argv[1];
	parsedoc(docname);
	return 1;
}

static void parsedoc(char* docname)
{
	xmlDocPtr doc;
	xmlNodePtr cur;

	doc = xmlParseFile(docname);
	if(doc == NULL)
	{
		fprintf(stderr, "Document not parsed successfully. \n");
		return;
	}

	cur = xmlDocGetRootElement(doc);
	if(cur == NULL)
	{
		fprintf(stderr, "empty document\n");
		xmlFree(doc);
		return;
	}
	
	if(xmlStrcmp(cur->name, (const xmlChar*)"data"))
	{
		fprintf(stderr, "document of the wrong type, root node != data\n");
		xmlFree(doc);
		return;
	}
	
	cur = cur->xmlChildrenNode;
	while(cur!=NULL)
	{
		xmlChar* attr;
		printf("-%s-\n", cur->name);
		if(!xmlStrcmp(cur->name, (const xmlChar*)"country"))
		{
			attr = xmlGetProp(cur, "name");
			printf("country name : %s\n", attr);
			//parseinfo(doc, cur);
			xmlFree(attr);
		}
		cur = cur->next;
	}
	
	xmlFreeDoc(doc);
	return;
}

static void parseinfo(xmlDocPtr doc, xmlNodePtr cur)
{
	cur = cur->xmlChildrenNode;
	while(cur!=NULL)
	{
		xmlChar* content;
		if(xmlStrcmp(cur->name, (const xmlChar*)"text"))
		{
			content = xmlNodeGetContent(cur);
			if(xmlStrcmp(content, (const xmlChar*)""))
			{
				printf("%s : %s\n", cur->name, content);
				xmlFree(content);
			}
		}
		cur = cur->next;
	}
	return;
}


