#include <stdio.h>
#include <stdlib.h>
#include "../inc/device.h"
#include "../inc/device_xml.h"
#include "../inc/logger.h"

static device_xml_t *alloc_devices_xml_file(void);
static void free_devices_xml_file(device_xml_t *);

static int read_device_type(device_t *device, xmlXPathContext *);
static int read_device_name(device_t *device, xmlXPathContext *);
static int read_device_description(device_t *device, xmlXPathContext *);
static int read_device_historify(device_t *device, xmlXPathContext *);
static int read_device_fire(device_t *device, xmlXPathContext *);
static int read_device_fire_date(device_t *device, xmlXPathContext *);
static int read_device_relay(device_t *device, xmlXPathContext *);
static int read_device_digital_input(device_t *device, xmlXPathContext *);
static int read_device_modbus(device_t *device, xmlXPathContext *);

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
		LOG_ERROR("Could not parse devices configuration xml file at \"%s\"",XML_DEVICES_PATH);
		return NULL;
	}

	dxml->xpath_context = xmlXPathNewContext(dxml->devices_doc);
	if(dxml->xpath_context == NULL){
		LOG_ERROR("Error: Unable to create new XPath context.");
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

xmlNode *read_devices_xml_by_id(int id){
	device_xml_t *dxml = open_devices_xml_file();

	device_t dev_ptr;
	xmlNode *found_device = NULL;

	int dev_idx = 0;
	for(xmlNode *device = dxml->root->children; device != NULL; device = device->next){
		if(device->type == XML_ELEMENT_NODE){
			if(dev_idx >= MAX_DEVICES) break;

			dxml->xpath_context->node = device;
			
			dev_ptr.id = read_device_id(device);

			if(dev_ptr.id == id){
				found_device = device;
			}
		}
	}

	close_devices_xml_file(dxml);

	return found_device;
}

int read_devices_xml(){
	// printf("Reading devices xml...\n");
	device_xml_t *dxml = open_devices_xml_file();

	if(dxml == NULL) return -1;

	int dev_idx = 0;
	for(xmlNode *device = dxml->root->children; device != NULL; device = device->next){
		if(device->type == XML_ELEMENT_NODE){
			if(dev_idx >= MAX_DEVICES) break;

			dxml->xpath_context->node = device;

            device_t *devices = get_devices_arr();
			struct Device *dev_ptr = &devices[dev_idx];
			dev_ptr->id = read_device_id(device);
			read_device_type(dev_ptr, dxml->xpath_context);
			read_device_name(dev_ptr, dxml->xpath_context);
			read_device_description(dev_ptr, dxml->xpath_context);
			read_device_historify(dev_ptr, dxml->xpath_context);

			if(strcmp(dev_ptr->type, "ANALYZER") != 0){
				read_device_fire(dev_ptr, dxml->xpath_context);
			}

			read_device_relay(dev_ptr, dxml->xpath_context);
			read_device_digital_input(dev_ptr, dxml->xpath_context);

			if(strcmp(dev_ptr->type, "ANALYZER") == 0){
				read_device_modbus(dev_ptr, dxml->xpath_context);
			}

			dev_idx++;
		}
	}

	close_devices_xml_file(dxml);

	// printf("Finished reading xml\n");
	
	return 0;
}

char *read_node_prop(xmlNode *node, const char *prop){
	xmlChar *_prop = xmlGetProp(node, BAD_CAST prop);

	return (char *)_prop;
}

char *read_node_content(const char *xpath_expression, xmlXPathContext *xpath_ctx){
	xmlXPathObjectPtr xpath_obj = xmlXPathEvalExpression(BAD_CAST xpath_expression, xpath_ctx);

	xmlNode *node = xpath_obj->nodesetval->nodeTab[0];
	xmlChar *content = xmlNodeGetContent(node);

	char *ret = strdup((char *)content);

	xmlFree(content);
	xmlXPathFreeObject(xpath_obj);

	return ret;
}

int read_device_id(xmlNode *dev_node){
	char *id = read_node_prop(dev_node, "id");
	return atoi(id);
}

static int read_device_type(device_t *device, xmlXPathContext *xpath_ctx){
	device->type = read_node_content("./type", xpath_ctx);
	return 0;
}

static int read_device_name(device_t *device, xmlXPathContext *xpath_ctx){
	device->name = read_node_content("./name", xpath_ctx);
	return 0;
}

static int read_device_description(device_t *device, xmlXPathContext *xpath_ctx){
	device->description = read_node_content("./description", xpath_ctx);
	return 0;
}

static int read_device_historify(device_t *device, xmlXPathContext *xpath_ctx){
	char *expr = "./historify";
	xmlXPathObjectPtr xpath_obj = xmlXPathEvalExpression(BAD_CAST expr, xpath_ctx);
	xmlNode *node = xpath_obj->nodesetval->nodeTab[0];

	char *active = read_node_prop(node, "active");
	char *period = read_node_prop(node, "period");

	device->hist.active = atoi(active);
	device->hist.period = atoi(period);

	xmlXPathFreeObject(xpath_obj);

	return 0;
}

static int read_device_fire(device_t *device, xmlXPathContext *xpath_ctx){
	char *expr = "./fire";
	xmlXPathObjectPtr xpath_obj = xmlXPathEvalExpression(BAD_CAST expr, xpath_ctx);

	xmlNode *node = xpath_obj->nodesetval->nodeTab[0];
	char *active = read_node_prop(node, "active");
	char *period = read_node_prop(node, "period");

	device->fire.active = atoi(active);
	device->fire.period = atoi(period);

	read_device_fire_date(device, xpath_ctx);

	xmlXPathFreeObject(xpath_obj);

	return 0;
}

static int read_device_fire_date(device_t *device, xmlXPathContext *xpath_ctx){
	xmlXPathObjectPtr xpath_obj_date = xmlXPathEvalExpression(BAD_CAST "./fire/date", xpath_ctx);
	xmlNode *date_node = xpath_obj_date->nodesetval->nodeTab[0];

	int i = 0;
	for(xmlNode *entry_node = date_node->children; entry_node != NULL; entry_node = entry_node->next){
		if(entry_node->type == XML_ELEMENT_NODE){
			if(i >= DATE_ENTRIES) break;

			device->fire.date.entries[i].date     = (char *)xmlNodeGetContent(entry_node);
			device->fire.date.entries[i].period   = atoi(read_node_prop(entry_node, "period"));
			device->fire.date.entries[i].duration = atoi(read_node_prop(entry_node, "duration"));
			device->fire.date.entries[i].start    = atoi(read_node_prop(entry_node, "start"));

			i++;
		}
	}
	xmlXPathFreeObject(xpath_obj_date);

	return 0;
}

static int read_device_relay(device_t *device, xmlXPathContext *xpath_ctx){
	xmlXPathObjectPtr xpath_obj_relay = xmlXPathEvalExpression(BAD_CAST "./relay", xpath_ctx);
	if(xpath_obj_relay && !xmlXPathNodeSetIsEmpty(xpath_obj_relay->nodesetval)){
		xmlNode *node_relay = xpath_obj_relay->nodesetval->nodeTab[0];

		device->rl.id_pin = read_node_prop(node_relay, "id_pin");
		device->rl.pin    = read_node_prop(node_relay, "pin");
	}
	if(xpath_obj_relay) xmlXPathFreeObject(xpath_obj_relay);

	return 0;
}

static int read_device_digital_input(device_t *device, xmlXPathContext *xpath_ctx){
	xmlXPathObjectPtr xpath_obj_relay = xmlXPathEvalExpression(BAD_CAST "./digital_input", xpath_ctx);
	if(xpath_obj_relay && !xmlXPathNodeSetIsEmpty(xpath_obj_relay->nodesetval)){
		xmlNode *node = xpath_obj_relay->nodesetval->nodeTab[0];

		device->di.id_pin = read_node_prop(node, "id_pin");
		device->di.pin    = read_node_prop(node, "pin");
	}
	if(xpath_obj_relay) xmlXPathFreeObject(xpath_obj_relay);

	return 0;
}

static int read_device_modbus(device_t *device, xmlXPathContext *xpath_ctx){
	xmlXPathObjectPtr xpath_obj_mb = xmlXPathEvalExpression(BAD_CAST "./modbus", xpath_ctx);
	if(xpath_obj_mb && !xmlXPathNodeSetIsEmpty(xpath_obj_mb->nodesetval)){
		xmlNode *node_mb = xpath_obj_mb->nodesetval->nodeTab[0];

		char *mb_con_type = read_node_prop(node_mb, "connection_type");
		if(strcmp(mb_con_type, "TCP") == 0)
			device->mb.connection_type = TCP;
		else if(strcmp(mb_con_type, "RS485") == 0)
			device->mb.connection_type = RS485;
		else {
			LOG_ERROR("Invalid modbus connection type \"%s\". Can not read modbus node in config xml file.", mb_con_type);
			return -1;
		}
		device->mb.slave = atoi(read_node_prop(node_mb, "slave"));
	}
	if(xpath_obj_mb) xmlXPathFreeObject(xpath_obj_mb);

	return 0;
}
