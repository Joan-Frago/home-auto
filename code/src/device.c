#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../inc/unipi_control.h"
#include "../inc/device.h"
#include "../inc/util.h"
#include "../inc/tcp_server.h"
#include "../inc/device_xml.h"
#include "../inc/logger.h"
#include "../inc/sb.h"
#include "../inc/config.h"

device_t devices[MAX_DEVICES];

static xmlNode *read_devices_xml_by_id(int id);
static int read_devices_xml();

static char *read_node_prop(xmlNode *dev_node, const char *prop);
static int read_device_id(xmlNode *);
static int read_device_type(device_t *device, xmlXPathContext *);
static int read_device_name(device_t *device, xmlXPathContext *);
static int read_device_description(device_t *device, xmlXPathContext *);
static int read_device_historify(device_t *device, xmlXPathContext *);
static int read_device_fire(device_t *device, xmlXPathContext *);
static int read_device_fire_date(device_t *device, xmlXPathContext *);
static int read_device_relay(device_t *device, xmlXPathContext *);
static int read_device_digital_input(device_t *device, xmlXPathContext *);
static int read_device_modbus(device_t *device, xmlXPathContext *);

/*
 * Set all devices before running.
 * Read from devices xml and init Devices.
 */
int set_devices(){
	LOG_INFO("Setting devices...");

	// dynamic configuration
	if(read_devices_xml() == -1){
		LOG_ERROR("Error: Could not read devices xml.");
		return -1;
	}

	// static configuration
	int i;
	for(i=0; i<MAX_DEVICES; i++){
		if(devices[i].type == NULL) continue;
		
		devices[i].hist.remaining_ticks = devices[i].hist.period;

		devices[i].fire.period = devices[i].fire.period * 3600;
		devices[i].fire.remaining_ticks = devices[i].fire.period;
		
		int j;
		for(j=0; j<DATE_ENTRIES; j++){
			devices[i].fire.date.entries[j].running = 0;

			devices[i].fire.date.entries[j].period = devices[i].fire.date.entries[j].period * 3600;
			devices[i].fire.date.entries[j].period_countdown = devices[i].fire.date.entries[j].period;

			devices[i].fire.date.entries[j].duration = devices[i].fire.date.entries[j].duration * 3600;
			devices[i].fire.date.entries[j].duration_countdown = devices[i].fire.date.entries[j].duration;
		}

		if(devices[i].rl.id_pin){
			devices[i].has_rl = 1;
			devices[i].rl.value = relay_read(&devices[i].rl);
			devices[i].rl.last_value = devices[i].rl.value;
		}
		else {
			devices[i].has_rl = 0;
		}

		if(devices[i].di.id_pin){
			devices[i].has_di = 1;
			devices[i].di.value = digital_read(&devices[i].di);
			devices[i].di.last_value = devices[i].di.value;
		}
		else {
			devices[i].has_di = 0;
		}

		if(strcmp(devices[i].type, "ANALYZER") == 0){
			devices[i].has_mb = 1;
			devices[i].mb.tcp_addr = get_var_value(MODBUS_TCP_ADDR);
			devices[i].mb.tcp_port = atoi(get_var_value(MODBUS_TCP_PORT));

			analyzer_set_registers(devices[i].mb.registers);

			int k;
			for(k=0; k<REGISTER_COUNT; k++){
				devices[i].mb.registers[k].value = modbus_read(devices[i].mb, devices[i].mb.registers[k]);
				devices[i].mb.registers[k].last_value = devices[i].mb.registers[k].value;
			}
		}
		else{
			devices[i].has_mb = 0;
		}
	}

	LOG_INFO("All devices have been set.");
	return 0;
}

static int read_devices_xml(){
	// printf("Reading devices xml...\n");
	device_xml_t *dxml = open_devices_xml_file();

	if(dxml == NULL) return -1;

	int dev_idx = 0;
	for(xmlNode *device = dxml->root->children; device != NULL; device = device->next){
		if(device->type == XML_ELEMENT_NODE){
			if(dev_idx >= MAX_DEVICES) break;

			dxml->xpath_context->node = device;

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

static xmlNode *read_devices_xml_by_id(int id){
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

static char *read_node_prop(xmlNode *node, const char *prop){
	xmlChar *_prop = xmlGetProp(node, BAD_CAST prop);

	return (char *)_prop;
}

static int read_device_id(xmlNode *dev_node){
	char *id = read_node_prop(dev_node, "id");

	return atoi(id);
}

static int read_device_type(device_t *device, xmlXPathContext *xpath_ctx){
	char *expr = "./type";
	xmlXPathObjectPtr xpath_obj = xmlXPathEvalExpression(BAD_CAST expr, xpath_ctx);

	xmlNode *node = xpath_obj->nodesetval->nodeTab[0];
	xmlChar *content = xmlNodeGetContent(node);

	device->type = strdup((char *)content);

	xmlFree(content);
	xmlXPathFreeObject(xpath_obj);

	return 0;
}

static int read_device_name(device_t *device, xmlXPathContext *xpath_ctx){
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

static int read_device_description(device_t *device, xmlXPathContext *xpath_ctx){
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

static int read_device_historify(device_t *device, xmlXPathContext *xpath_ctx){
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

static int read_device_fire(device_t *device, xmlXPathContext *xpath_ctx){
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

		device->rl.id_pin = (char *)xmlGetProp(node_relay, BAD_CAST "id_pin");
		device->rl.pin = (char *)xmlGetProp(node_relay, BAD_CAST "pin");
		//printf("Device [%d] relay: id_pin=%s pin=%s\n", device->id, device->rl.id_pin, device->rl.pin);
	}
	if(xpath_obj_relay) xmlXPathFreeObject(xpath_obj_relay);

	return 0;
}

static int read_device_digital_input(device_t *device, xmlXPathContext *xpath_ctx){
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

static int read_device_modbus(device_t *device, xmlXPathContext *xpath_ctx){
	xmlXPathObjectPtr xpath_obj_mb = xmlXPathEvalExpression(BAD_CAST "./modbus", xpath_ctx);
	if(xpath_obj_mb && !xmlXPathNodeSetIsEmpty(xpath_obj_mb->nodesetval)){
		xmlNode *node_mb = xpath_obj_mb->nodesetval->nodeTab[0];

		char *mb_con_type = (char *)xmlGetProp(node_mb, BAD_CAST "connection_type");
		if(strcmp(mb_con_type, "TCP") == 0){
			device->mb.connection_type = TCP;
		}
		else if(strcmp(mb_con_type, "RS485") == 0){
			device->mb.connection_type = RS485;
		}
		else {
			LOG_ERROR("Invalid modbus connection type \"%s\". Can not read modbus node in config xml file.", mb_con_type);
			return -1;
		}

		device->mb.slave = atoi((char *)xmlGetProp(node_mb, BAD_CAST "slave"));
	}
	if(xpath_obj_mb) xmlXPathFreeObject(xpath_obj_mb);

	return 0;
}

device_t *get_devices_arr(void){
	return devices;
}

/*
 * Returns a pointer to the device with the id provided as argument.
 * If not found, returns NULL;
 */
device_t *get_device_by_id(int id){
	int i;
	for(i=0; i<MAX_DEVICES; i++){
		if(devices[i].id == id){
			return &devices[i];
		}
	}
	return NULL;
}

int get_all_devices(char *resp_buf){
	FILE *fptr = fopen(XML_DEVICES_PATH, "r");
	if(fptr==NULL){
		LOG_ERROR("Error: Could not open %s",XML_DEVICES_PATH);
		return -1;
	}

	char c;
	int i = 0;
	while((c = getc(fptr)) != EOF && i < MESSAGE_SIZE-1){
		resp_buf[i++] = c;
	}
	resp_buf[i] = '\0';
	// printf("Response:\n%s\n",resp_buf);

	return 0;
}

int set_device(xmlNode *dev_node){
	LOG_DEBUG("Setting device...\n");

	return 0;
}

int get_device(char *resp_buf, xmlNode *data){
	xmlNode *tmp_node = find_child_node(data, BAD_CAST "device");
	if(tmp_node == NULL){
		LOG_ERROR("Error: device.c : Did not find a child node called \"device\"");
		return -1;
	}

	device_t tmp_dev;
	tmp_dev.id = read_device_id(tmp_node);

	device_t *device = get_device_by_id(tmp_dev.id);
	if(device == NULL)
		return -1;

	// Construct response
	StringBuilder *sb = sb_create();

	sb_appendf(sb, "<device id=\"%d\" type=\"%s\">", device->id, device->type);
	sb_appendf(sb, "<name>%s</name>", device->name);
	sb_appendf(sb, "<description>%s</description>", device->description);

	if(device->has_rl == 1)
		sb_appendf(sb, "<relay id_pin=\"%s\" pin=\"%s\" value=\"%d\"></relay>", device->rl.id_pin, device->rl.pin, device->rl.value);

	if(device->has_di == 1)
		sb_appendf(sb, "<digital_input id_pin=\"%s\" pin=\"%s\" value=\"%d\"></digital_input>", device->di.id_pin, device->di.pin, device->di.value);

	if(device->has_mb == 1){
		sb_append(sb, "<modbus>");
		int i;
		for(i=0; i<REGISTER_COUNT; i++){
			sb_appendf(
				sb,
				"<register name=\"%s\" symbol=\"%s\" line=\"%s\" value=\"%f\"></register>",
				device->mb.registers[i].name,
				device->mb.registers[i].symbol,
				device->mb.registers[i].line,
				device->mb.registers[i].value
			);
		}
		sb_append(sb, "</modbus>");
	}
	
	sb_append(sb, "</device>");

	char *temp = sb_concat(sb);
	if(temp){
		strncpy(resp_buf, temp, MESSAGE_SIZE);
		free(temp);
	}
	sb_free(sb);
	
	return 0;
}

int update_pin_state(char *resp_buf, xmlNode *data){
	xmlNode *tmp_node = find_child_node(data, BAD_CAST "device");
	if(tmp_node == NULL){
		LOG_ERROR("Error: device.c : Did not find a child node called \"device\"");
		return -1;
	}

	device_t tmp_dev;
	tmp_dev.id = read_device_id(tmp_node);

	char *new_state = read_node_prop(tmp_node, "new_state");
	if(new_state == NULL){
		LOG_ERROR("Error: device.c : Could not read new_state property from node");
		return -1;
	}

	device_t *device = get_device_by_id(tmp_dev.id);
	if(device == NULL)
		return -1;

	relay_write(&device->rl, atoi(new_state));
	// it is independent of historification so I don't need to manually update rl.value nor rl.last_value

	char *temp = "<update><status>ok</status></update>";
	strncpy(resp_buf, temp, MESSAGE_SIZE);

	return 0;
}
