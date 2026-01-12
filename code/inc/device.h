#include "./unipi_control.h"

#ifndef DEVICE_H

#define DEVICE_H

#define MAX_DEVICES 24
#define DEVICE_NAME_SIZE 64
#define DEVICE_DESC_SIZE 1024

#define XML_DEVICES_PATH "conf/devices.xml"

typedef struct Device{
	int id;
	char *name;
	char *description;
	historify_t hist;
	fire_device_t fire;

	rl_t rl;
	di_t di;
} device_t;

int set_devices(struct Device devices[MAX_DEVICES]);

#include <libxml2/libxml/tree.h>
#include <libxml2/libxml/xpath.h>

int read_devices_xml(struct Device devices[MAX_DEVICES]);
int read_device_id(struct Device *device, xmlNode *);
int read_device_name(struct Device *device, xmlXPathContext *);
int read_device_description(struct Device *device, xmlXPathContext *);
int read_device_historify(struct Device *device, xmlXPathContext *);
int read_device_fire(struct Device *device, xmlXPathContext *);
int read_device_fire_date(struct Device *device, xmlXPathContext *);
int read_device_relay(struct Device *device, xmlXPathContext *);
int read_device_digital_input(struct Device *device, xmlXPathContext *);

int get_all_devices(char *resp_buf);
int set_device(xmlNode *);

#endif
