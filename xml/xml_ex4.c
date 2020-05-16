

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

int main()
{
	xmlDocPtr doc;
	xmlNodePtr cur;
	xmlChar* content;
	
	char ip[10][20];

	int i = 0;

	doc = xmlParseFile("ip.xml");
	if(doc == NULL)
	{
		fprintf(stderr, "document not parsed successfully.\n");
		return 0;
	}

	cur = xmlDocGetRootElement(doc);

	if(cur == NULL)
	{
		fprintf(stderr, "empty document.\n");
		xmlFreeDoc(doc);
		
		return 0;
	}

	cur = cur->xmlChildrenNode;

	while(cur != NULL)
	{
		if(!xmlStrcmp(cur->name, (const xmlChar*)"ip"))
		{
			content = xmlNodeGetContent(cur);
			strncpy(ip[i], content, sizeof(ip[0]));

			i++;
			xmlFree(content);
		}

		cur = cur->next;
	}

	for(int j = 0; j<i; j++)
		printf("%s\n", ip[j]);
	
	xmlFreeDoc(doc);

	return 0;
}
