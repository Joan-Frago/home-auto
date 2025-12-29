#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libxml2/libxml/parser.h>
#include <libxml2/libxml/xpath.h>
#include <libxml2/libxml/xpathInternals.h>
#include "../inc/unipi_control.h"

int read_devices_xml(struct Device devices[MAX_DEVICES]);
int read_device_name(struct Device devices[MAX_DEVICES], xmlXPathContext *, int);
int read_device_description(struct Device devices[MAX_DEVICES], xmlXPathContext *, int);

/*
 * Set all devices before running.
 * Read from devices xml and init Devices.
 */
int set_devices(struct Device devices[MAX_DEVICES]){
	printf("Setting devices...\n");

	if(read_devices_xml(devices) == -1){
		printf("Error: Could not read devices xml.\n");
		return -1;
	}

	for(int i = 0; i < MAX_DEVICES; i++){
		devices[i].rl = malloc(sizeof(struct Relay));
		devices[i].di = malloc(sizeof(struct DigitalInput));

		if(i <= 8){
			//char *id_pin = malloc(sizeof("RO2.1"));
			//char *pin = malloc(sizeof("2.1"));
			//pin = ""
			devices[i].rl->id_pin = "RO2.1";
			devices[i].di->id_pin = "DI1.1";
		}
		else if(i <= 16){
			devices[i].di->id_pin = "DI2.1";
		}
	}

	printf("All devices have been set.\n\n");
	return 0;
}

int read_devices_xml(struct Device devices[MAX_DEVICES]){
	xmlDoc *devices_doc = xmlReadFile(XML_DEVICES_PATH, NULL, 0);
	if(devices_doc == NULL){
		printf("Could not parse devices configuration xml file at \"%s\"\n",XML_DEVICES_PATH);
		return -1;
	}

	xmlXPathContext *xpath_ctx = xmlXPathNewContext(devices_doc);
	if(xpath_ctx == NULL){
		printf("Error: Unable to create new XPath context.\n");
		return -1;
	}

	xmlNode *root = xmlDocGetRootElement(devices_doc);

	int dev_idx = 0;
	for(xmlNode *device = root->children; device != NULL; device = device->next){
		if(device->type == XML_ELEMENT_NODE){
			if(dev_idx >= MAX_DEVICES) break;

			xpath_ctx->node = device;

			read_device_name(devices, xpath_ctx, dev_idx);
			read_device_description(devices, xpath_ctx, dev_idx);

			dev_idx++;
		}
	}

	xmlFreeDoc(devices_doc);
	xmlXPathFreeContext(xpath_ctx);

	printf("Finished reading xml\n");
	
	return 0;
}

int read_device_name(struct Device devices[MAX_DEVICES], xmlXPathContext *xpath_ctx, int dev_idx){
	char *expr = "./name";
	xmlXPathObjectPtr xpath_obj = xmlXPathEvalExpression(BAD_CAST expr, xpath_ctx);
	if(xpath_obj && !xmlXPathNodeSetIsEmpty(xpath_obj->nodesetval)){
		xmlNode *node = xpath_obj->nodesetval->nodeTab[0];
		xmlChar *content = xmlNodeGetContent(node);

		devices[dev_idx].name = strdup((char *)content);

		printf("Device [%d] name: %s\n", dev_idx, devices[dev_idx].name);

		xmlFree(content);
	}

	if(xpath_obj) xmlXPathFreeObject(xpath_obj);

	return 0;
}

int read_device_description(struct Device devices[MAX_DEVICES], xmlXPathContext *xpath_ctx, int dev_idx){
	char *expr = "./description";
	xmlXPathObjectPtr xpath_obj = xmlXPathEvalExpression(BAD_CAST expr, xpath_ctx);
	if(xpath_obj && !xmlXPathNodeSetIsEmpty(xpath_obj->nodesetval)){
		xmlNode *node = xpath_obj->nodesetval->nodeTab[0];
		xmlChar *content = xmlNodeGetContent(node);

		devices[dev_idx].description = strdup((char *)content);

		printf("Device [%d] description: %s\n", dev_idx, devices[dev_idx].description);

		xmlFree(content);
	}

	if(xpath_obj) xmlXPathFreeObject(xpath_obj);

	return 0;
}
