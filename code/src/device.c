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
