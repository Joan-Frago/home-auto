#include <stdio.h>
#include <string.h>
#include <libxml2/libxml/parser.h>
#include <libxml2/libxml/xpath.h>
#include <libxml2/libxml/xpathInternals.h>
#include "../inc/unipi_control.h"
#include "../inc/device.h"
#include "../inc/util.h"

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

			struct Device *dev_ptr = &devices[dev_idx];
			read_device_id(dev_ptr, device);
			read_device_name(dev_ptr, xpath_ctx);
			read_device_description(dev_ptr, xpath_ctx);
			read_device_historify(dev_ptr, xpath_ctx);
			read_device_fire(dev_ptr, xpath_ctx);
			read_device_relay(dev_ptr, xpath_ctx);
			read_device_digital_input(dev_ptr, xpath_ctx);

			dev_idx++;
		}
	}

	xmlFreeDoc(devices_doc);
	xmlXPathFreeContext(xpath_ctx);

	printf("Finished reading xml\n");
	
	return 0;
}

int read_device_id(struct Device *device, xmlNode *dev_node){
	xmlChar *id = xmlGetProp(dev_node, BAD_CAST "id");
	device->id = char2int((char *)id);
	xmlFree(id);

	printf("Device [%d]\n", device->id);

	return 0;
}

int read_device_name(struct Device *device, xmlXPathContext *xpath_ctx){
	char *expr = "./name";
	xmlXPathObjectPtr xpath_obj = xmlXPathEvalExpression(BAD_CAST expr, xpath_ctx);

	xmlNode *node = xpath_obj->nodesetval->nodeTab[0];
	xmlChar *content = xmlNodeGetContent(node);

	device->name = strdup((char *)content);

	printf("Device [%d] name: %s\n", device->id, device->name);

	xmlFree(content);
	xmlXPathFreeObject(xpath_obj);

	return 0;
}

int read_device_description(struct Device *device, xmlXPathContext *xpath_ctx){
	char *expr = "./description";
	xmlXPathObjectPtr xpath_obj = xmlXPathEvalExpression(BAD_CAST expr, xpath_ctx);

	xmlNode *node = xpath_obj->nodesetval->nodeTab[0];
	xmlChar *content = xmlNodeGetContent(node);

	device->description = strdup((char *)content);

	printf("Device [%d] description: %s\n", device->id, device->description);

	xmlFree(content);
	xmlXPathFreeObject(xpath_obj);

	return 0;
}

int read_device_historify(struct Device *device, xmlXPathContext *xpath_ctx){
	char *expr = "./historify";
	xmlXPathObjectPtr xpath_obj = xmlXPathEvalExpression(BAD_CAST expr, xpath_ctx);

	xmlNode *node = xpath_obj->nodesetval->nodeTab[0];
	xmlChar *active = xmlGetProp(node, BAD_CAST "active");
	xmlChar *period = xmlGetProp(node, BAD_CAST "period"); // Period must be between 0 and 9

	device->hist.active = char2int((char *)active);
	device->hist.period = char2int((char *)period);

	printf("Device [%d] historify: active=%d period=%d\n", device->id, device->hist.active, device->hist.period);

	xmlFree(active);
	xmlFree(period);
	xmlXPathFreeObject(xpath_obj);

	return 0;
}

int read_device_fire(struct Device *device, xmlXPathContext *xpath_ctx){
	char *expr = "./fire";
	xmlXPathObjectPtr xpath_obj = xmlXPathEvalExpression(BAD_CAST expr, xpath_ctx);

	xmlNode *node = xpath_obj->nodesetval->nodeTab[0];
	xmlChar *active = xmlGetProp(node, BAD_CAST "active");
	xmlChar *period = xmlGetProp(node, BAD_CAST "period");

	device->fire.active = char2int((char *)active);
	device->fire.period = char2int((char *)period);

	printf("Device [%d] fire: active=%d period=%d\n", device->id, device->fire.active, device->fire.period);

	read_device_fire_date(device, xpath_ctx);

	xmlFree(active);
	xmlFree(period);
	xmlXPathFreeObject(xpath_obj);

	return 0;
}

int read_device_fire_date(struct Device *device, xmlXPathContext *xpath_ctx){
	xmlXPathObjectPtr xpath_obj_start = xmlXPathEvalExpression(BAD_CAST "./fire/date/start", xpath_ctx);
	xmlNode *date_start = xpath_obj_start->nodesetval->nodeTab[0];

	xmlXPathObjectPtr xpath_obj_end = xmlXPathEvalExpression(BAD_CAST "./fire/date/end", xpath_ctx);
	xmlNode *date_end = xpath_obj_end->nodesetval->nodeTab[0];

	device->fire.date.start = (char *)xmlNodeGetContent(date_start);
	device->fire.date.end = (char *)xmlNodeGetContent(date_end);

	printf("Device [%d] fire date: start=%s end=%s\n", device->id, device->fire.date.start, device->fire.date.end);

	xmlXPathFreeObject(xpath_obj_start);
	xmlXPathFreeObject(xpath_obj_end);

	return 0;
}

int read_device_relay(struct Device *device, xmlXPathContext *xpath_ctx){
	xmlXPathObjectPtr xpath_obj_relay = xmlXPathEvalExpression(BAD_CAST "./relay", xpath_ctx);
	if(xpath_obj_relay && !xmlXPathNodeSetIsEmpty(xpath_obj_relay->nodesetval)){
		xmlNode *node_relay = xpath_obj_relay->nodesetval->nodeTab[0];

		device->rl.id_pin = (char *)xmlGetProp(node_relay, BAD_CAST "id_pin");
		device->rl.pin = (char *)xmlGetProp(node_relay, BAD_CAST "pin");

		printf("Device [%d] relay: id_pin=%s pin=%s\n", device->id, device->rl.id_pin, device->rl.pin);
	}
	if(xpath_obj_relay) xmlXPathFreeObject(xpath_obj_relay);

	return 0;
}

int read_device_digital_input(struct Device *device, xmlXPathContext *xpath_ctx){
	xmlXPathObjectPtr xpath_obj_relay = xmlXPathEvalExpression(BAD_CAST "./digital_input", xpath_ctx);
	if(xpath_obj_relay && !xmlXPathNodeSetIsEmpty(xpath_obj_relay->nodesetval)){
		xmlNode *node_relay = xpath_obj_relay->nodesetval->nodeTab[0];

		device->di.id_pin = (char *)xmlGetProp(node_relay, BAD_CAST "id_pin");
		device->di.pin = (char *)xmlGetProp(node_relay, BAD_CAST "pin");

		printf("Device [%d] digital_input: id_pin=%s pin=%s\n", device->id, device->di.id_pin, device->di.pin);
	}
	if(xpath_obj_relay) xmlXPathFreeObject(xpath_obj_relay);

	return 0;
}

int get_all_devices(void){
	printf("Getting all devices...\n");

	return 0;
}

int set_device(xmlNode *dev_node){
	printf("Setting device...\n");

	return 0;
}
