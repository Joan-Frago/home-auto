#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <libxml2/libxml/parser.h>
#include <libxml2/libxml/xpath.h>
#include <libxml2/libxml/xpathInternals.h>

#include "../inc/config.h"
#include "../inc/tcp_server.h"
#include "../inc/unipi_control.h"

#define XML_DEVICES_PATH "conf/devices.xml"
#define XML_XPATH_EXPR_SIZE 255

int set_devices(void);
int read_devices_xml();
int set_device_name(xmlXPathContext *, int);
int set_device_description(xmlXPathContext *, int);

void exit_handler();

struct Device devices[MAX_DEVICES];

int main(){
	signal(SIGINT, exit_handler);
	signal(SIGTERM, exit_handler);

	printf("Application PID is %ld\n\n",(long)getpid());

	if(load_config() == -1){
		perror("load_config returned -1. Error loading configuration\n");
		return -1;
	}

	// Set all devices before running (read from db and init Devices (Relay or Digital Input or Both))
	if(set_devices() == -1){
		printf("Error: set_devices function returned with error code -1\n");
		return -1;
	}

	// Start a new thread for each functionality
	pthread_t core_thread;
	pthread_create(&core_thread, NULL, core, NULL);

	pthread_t tcp_server_thread;
	pthread_create(&tcp_server_thread, NULL, start_tcp_server, NULL);

	pthread_join(core_thread, NULL);
	pthread_join(tcp_server_thread, NULL);

	return 0;
}

void exit_handler(){
	printf("\nProgram terminated\n");
	printf("Exiting application...\n");

	exit(0);
}

/*
 * Set all devices before running.
 * Read from devices xml and init Devices.
 */
int set_devices(){
	printf("Setting devices...\n");

	if(read_devices_xml() == -1){
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

int read_devices_xml(){
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

			set_device_name(xpath_ctx, dev_idx);
			set_device_description(xpath_ctx, dev_idx);

			dev_idx++;
		}
	}

	xmlFreeDoc(devices_doc);
	xmlXPathFreeContext(xpath_ctx);

	printf("Finished reading xml\n");
	
	return 0;
}

int set_device_name(xmlXPathContext *xpath_ctx, int dev_idx){
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

int set_device_description(xmlXPathContext *xpath_ctx, int dev_idx){
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

/*
 * Infinite loop handling each Device's properties.
 * E.g. Historify and Date scheduling.
 * Only Devices must be used.
 * A single Relay or DigitalInput must not be used
 * since it has no logic attached to it.
 */
void *core(void* arg){
	printf("Core Thread ID is %lu\n",(unsigned long)pthread_self());

	// iterate through devices array and perform whatever actions have to be done

	return 0;
}
