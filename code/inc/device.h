#ifndef DEVICE_H

#include <stdint.h>

#include "./unipi_control.h"

#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>

#define DEVICE_H

#define MAX_DEVICES 24
#define DEVICE_NAME_SIZE 64
#define DEVICE_DESC_SIZE 1024

typedef struct Device{
	int id;
	int has_mb;
	int has_di;
	int has_rl;
	char *type;
	char *name;
	char *description;
	historify_t hist;
	fire_device_t fire;

	rl_t rl;
	di_t di;
	mb_t mb;
} device_t;

int set_devices();

device_t *get_devices_arr(void);
device_t *get_device_by_id(int id);

int get_all_devices(char *resp_buf);
int get_device_pin_status(char *resp_buf, xmlNode *);

int set_device(xmlNode *);
int get_device(char *resp_buf, xmlNode *data);

int update_pin_state(char *resp_buf, xmlNode *data);

#endif
