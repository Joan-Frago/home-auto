#include <stdio.h>
#include <stdlib.h>
#include "../inc/device_xml.h"

static device_xml_t *alloc_devices_xml_file(void);
static void free_devices_xml_file(device_xml_t *);

static device_xml_t *alloc_devices_xml_file(void){
	//printf("Allocating memory for devices xml file struct...\n");

	device_xml_t *dxml = (device_xml_t *) malloc(sizeof(device_xml_t));
	if(dxml == NULL) return NULL;

	dxml->xpath_context = NULL;
	dxml->devices_doc = NULL;
	dxml->root = NULL;

	//printf("Memory allocated for devices xml file struct.\n");

	return dxml;
}

static void free_devices_xml_file(device_xml_t *dxml){
	// printf("Freeing dxml...\n");

	if(dxml->xpath_context){
		xmlXPathFreeContext(dxml->xpath_context);
		// printf("dxml->xpath_context freed\n");
	}

	if(dxml->devices_doc){
		xmlFreeDoc(dxml->devices_doc);
		// printf("dxml->devices_doc freed\n");

		// root is freed by xmlFreeDoc
	}

	free(dxml);
	// printf("dxml freed\n");
}

device_xml_t *open_devices_xml_file(void){
	// printf("Opening devices xml file...\n");

	device_xml_t *dxml = alloc_devices_xml_file();
	if(dxml == NULL) return NULL;

	dxml->devices_doc = xmlReadFile(XML_DEVICES_PATH, NULL, 0);
	if(dxml->devices_doc == NULL){
		printf("Could not parse devices configuration xml file at \"%s\"\n",XML_DEVICES_PATH);
		return NULL;
	}

	dxml->xpath_context = xmlXPathNewContext(dxml->devices_doc);
	if(dxml->xpath_context == NULL){
		printf("Error: Unable to create new XPath context.\n");
		return NULL;
	}

	dxml->root = xmlDocGetRootElement(dxml->devices_doc);

	// printf("Devices file opened.\n");

	return dxml;
}

void close_devices_xml_file(device_xml_t *dxml){
	free_devices_xml_file(dxml);
}

xmlNode *find_child_node(xmlNode *parent, xmlChar *node_name) {
    xmlNodePtr cur = parent->xmlChildrenNode;
    while (cur != NULL) {
        if (xmlStrcmp(cur->name, node_name) == 0) {
            return cur;
        }
        cur = cur->next;
    }
    return NULL;
}
