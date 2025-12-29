#define MAX_DEVICES 24

#define DEVICE_NAME_SIZE 64
#define DEVICE_DESC_SIZE 1024

#define XML_DEVICES_PATH "conf/devices.xml"
#define XML_XPATH_EXPR_SIZE 255

typedef struct Relay{
	char *id_pin; // e.g. RO2.1
	char *pin; // e.g. 2.1
} rl_t;

typedef struct DigitalInput{
	char *id_pin;
	char *pin;
} di_t;

typedef struct Historify{
	int active;
	int period;
} historify_t;

typedef struct Date{
	char *start;
	char *end;
} date_t;

typedef struct Fire_Device{
	int active;
	int period;
	date_t date;

} fire_device_t;

typedef struct Device{
	char *name;
	char *description;
	historify_t *hist;
	fire_device_t fire;

	struct Relay *rl;
	struct DigitalInput *di;
} device_t;

int set_devices(struct Device devices[MAX_DEVICES]);

int relay_write(struct Relay *, int);
int digital_read(struct DigitalInput *, int *);

void *core(void*);
