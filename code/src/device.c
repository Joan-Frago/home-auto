#include <stdio.h>
#include <string.h>
#include "../inc/unipi_control.h"
#include "../inc/device.h"
#include "../inc/util.h"
#include "../inc/tcp_server.h"
#include "../inc/device_xml.h"

static xmlNode *read_devices_xml_by_id(int id);
static int read_devices_xml(struct Device devices[MAX_DEVICES]);

static int read_device_id(struct Device *device, xmlNode *);
static int read_device_name(struct Device *device, xmlXPathContext *);
static int read_device_description(struct Device *device, xmlXPathContext *);
static int read_device_historify(struct Device *device, xmlXPathContext *);
static int read_device_fire(struct Device *device, xmlXPathContext *);
static int read_device_fire_date(struct Device *device, xmlXPathContext *);
static int read_device_relay(struct Device *device, xmlXPathContext *);
static int read_device_digital_input(struct Device *device, xmlXPathContext *);


/*
 * Set all devices before running.
 * Read from devices xml and init Devices.
 */
int set_devices(device_t devices[MAX_DEVICES]){
	printf("Setting devices...\n");

	// dynamic configuration
	if(read_devices_xml(devices) == -1){
		printf("Error: Could not read devices xml.\n");
		return -1;
	}

	// static configuration
	int i;
	for(i=0; i<MAX_DEVICES; i++){
		devices[i].hist.remaining_ticks = devices[i].hist.period;
		devices[i].fire.remaining_ticks = devices[i].hist.period*3600;
	}

	printf("All devices have been set.\n\n");
	return 0;
}

static int read_devices_xml(struct Device devices[MAX_DEVICES]){
	// printf("Reading devices xml...\n");
	device_xml_t *dxml = open_devices_xml_file();

	if(dxml == NULL) return -1;

	int dev_idx = 0;
	for(xmlNode *device = dxml->root->children; device != NULL; device = device->next){
		if(device->type == XML_ELEMENT_NODE){
			if(dev_idx >= MAX_DEVICES) break;

			dxml->xpath_context->node = device;

			struct Device *dev_ptr = &devices[dev_idx];
			read_device_id(dev_ptr, device);
			read_device_name(dev_ptr, dxml->xpath_context);
			read_device_description(dev_ptr, dxml->xpath_context);
			read_device_historify(dev_ptr, dxml->xpath_context);
			read_device_fire(dev_ptr, dxml->xpath_context);
			read_device_relay(dev_ptr, dxml->xpath_context);
			read_device_digital_input(dev_ptr, dxml->xpath_context);

			dev_idx++;
		}
	}

	close_devices_xml_file(dxml);

	// printf("Finished reading xml\n");
	
	return 0;
}

static xmlNode *read_devices_xml_by_id(int id){
	device_xml_t *dxml = open_devices_xml_file();

	device_t dev_ptr;
	xmlNode *found_device = NULL;

	int dev_idx = 0;
	for(xmlNode *device = dxml->root->children; device != NULL; device = device->next){
		if(device->type == XML_ELEMENT_NODE){
			if(dev_idx >= MAX_DEVICES) break;

			dxml->xpath_context->node = device;
			
			read_device_id(&dev_ptr, device);

			if(dev_ptr.id == id){
				found_device = device;
			}
		}
	}

	close_devices_xml_file(dxml);

	return found_device;
}

static int read_device_id(struct Device *device, xmlNode *dev_node){
	xmlChar *id = xmlGetProp(dev_node, BAD_CAST "id");
	device->id = char2int((char *)id);
	//printf("Device [%d]\n", device->id);

	xmlFree(id);
	return 0;
}

static int read_device_name(struct Device *device, xmlXPathContext *xpath_ctx){
	char *expr = "./name";
	xmlXPathObjectPtr xpath_obj = xmlXPathEvalExpression(BAD_CAST expr, xpath_ctx);

	xmlNode *node = xpath_obj->nodesetval->nodeTab[0];
	xmlChar *content = xmlNodeGetContent(node);

	device->name = strdup((char *)content);
	//printf("Device [%d] name: %s\n", device->id, device->name);

	xmlFree(content);
	xmlXPathFreeObject(xpath_obj);

	return 0;
}

static int read_device_description(struct Device *device, xmlXPathContext *xpath_ctx){
	char *expr = "./description";
	xmlXPathObjectPtr xpath_obj = xmlXPathEvalExpression(BAD_CAST expr, xpath_ctx);

	xmlNode *node = xpath_obj->nodesetval->nodeTab[0];
	xmlChar *content = xmlNodeGetContent(node);

	device->description = strdup((char *)content);
	//printf("Device [%d] description: %s\n", device->id, device->description);

	xmlFree(content);
	xmlXPathFreeObject(xpath_obj);

	return 0;
}

static int read_device_historify(struct Device *device, xmlXPathContext *xpath_ctx){
	char *expr = "./historify";
	xmlXPathObjectPtr xpath_obj = xmlXPathEvalExpression(BAD_CAST expr, xpath_ctx);

	xmlNode *node = xpath_obj->nodesetval->nodeTab[0];
	xmlChar *active = xmlGetProp(node, BAD_CAST "active");
	xmlChar *period = xmlGetProp(node, BAD_CAST "period"); // Period must be between 0 and 9

	device->hist.active = char2int((char *)active);
	device->hist.period = char2int((char *)period);
	//printf("Device [%d] historify: active=%d period=%d\n", device->id, device->hist.active, device->hist.period);

	xmlFree(active);
	xmlFree(period);
	xmlXPathFreeObject(xpath_obj);

	return 0;
}

static int read_device_fire(struct Device *device, xmlXPathContext *xpath_ctx){
	char *expr = "./fire";
	xmlXPathObjectPtr xpath_obj = xmlXPathEvalExpression(BAD_CAST expr, xpath_ctx);

	xmlNode *node = xpath_obj->nodesetval->nodeTab[0];
	xmlChar *active = xmlGetProp(node, BAD_CAST "active");
	xmlChar *period = xmlGetProp(node, BAD_CAST "period");

	device->fire.active = char2int((char *)active);
	device->fire.period = char2int((char *)period);
	//printf("Device [%d] fire: active=%d period=%d\n", device->id, device->fire.active, device->fire.period);

	read_device_fire_date(device, xpath_ctx);

	xmlFree(active);
	xmlFree(period);
	xmlXPathFreeObject(xpath_obj);

	return 0;
}

static int read_device_fire_date(struct Device *device, xmlXPathContext *xpath_ctx){
	xmlXPathObjectPtr xpath_obj_start = xmlXPathEvalExpression(BAD_CAST "./fire/date/start", xpath_ctx);
	xmlNode *date_start = xpath_obj_start->nodesetval->nodeTab[0];

	xmlXPathObjectPtr xpath_obj_end = xmlXPathEvalExpression(BAD_CAST "./fire/date/end", xpath_ctx);
	xmlNode *date_end = xpath_obj_end->nodesetval->nodeTab[0];

	device->fire.date.start = (char *)xmlNodeGetContent(date_start);
	device->fire.date.end = (char *)xmlNodeGetContent(date_end);
	//printf("Device [%d] fire date: start=%s end=%s\n", device->id, device->fire.date.start, device->fire.date.end);

	xmlXPathFreeObject(xpath_obj_start);
	xmlXPathFreeObject(xpath_obj_end);

	return 0;
}

static int read_device_relay(struct Device *device, xmlXPathContext *xpath_ctx){
	xmlXPathObjectPtr xpath_obj_relay = xmlXPathEvalExpression(BAD_CAST "./relay", xpath_ctx);
	if(xpath_obj_relay && !xmlXPathNodeSetIsEmpty(xpath_obj_relay->nodesetval)){
		xmlNode *node_relay = xpath_obj_relay->nodesetval->nodeTab[0];

		device->rl.id_pin = (char *)xmlGetProp(node_relay, BAD_CAST "id_pin");
		device->rl.pin = (char *)xmlGetProp(node_relay, BAD_CAST "pin");
		//printf("Device [%d] relay: id_pin=%s pin=%s\n", device->id, device->rl.id_pin, device->rl.pin);
	}
	if(xpath_obj_relay) xmlXPathFreeObject(xpath_obj_relay);

	return 0;
}

static int read_device_digital_input(struct Device *device, xmlXPathContext *xpath_ctx){
	xmlXPathObjectPtr xpath_obj_relay = xmlXPathEvalExpression(BAD_CAST "./digital_input", xpath_ctx);
	if(xpath_obj_relay && !xmlXPathNodeSetIsEmpty(xpath_obj_relay->nodesetval)){
		xmlNode *node_relay = xpath_obj_relay->nodesetval->nodeTab[0];

		device->di.id_pin = (char *)xmlGetProp(node_relay, BAD_CAST "id_pin");
		device->di.pin = (char *)xmlGetProp(node_relay, BAD_CAST "pin");
		//printf("Device [%d] digital_input: id_pin=%s pin=%s\n", device->id, device->di.id_pin, device->di.pin);
	}
	if(xpath_obj_relay) xmlXPathFreeObject(xpath_obj_relay);

	return 0;
}

int get_all_devices(char *resp_buf){
	FILE *fptr = fopen(XML_DEVICES_PATH, "r");
	if(fptr==NULL){
		printf("Error: Could not open %s\n",XML_DEVICES_PATH);
		return -1;
	}

	char c;
	int i = 0;
	while((c = getc(fptr)) != EOF && i < MESSAGE_SIZE){
		resp_buf[i++] = c;
	}
	resp_buf[i] = '\0';
	// printf("Response:\n%s\n",resp_buf);

	return 0;
}

int set_device(xmlNode *dev_node){
	printf("Setting device...\n");

	return 0;
}
