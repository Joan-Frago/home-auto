#ifndef UNIPI_CONTROL_H
#define UNIPI_CONTROL_H

#define XML_XPATH_EXPR_SIZE 255

typedef struct Relay{
	char *id_pin; // e.g. RO2.1
	char *pin; // e.g. 2.1
	int value;
	int last_value;
} rl_t;

typedef struct DigitalInput{
	char *id_pin;
	char *pin;
	int value;
	int last_value;
} di_t;

typedef struct Historify{
	int active;
	int period; // number of seconds
	int remaining_ticks; // seconds left to historify
} historify_t;

typedef struct Date{
	char *start;
	char *end;
} date_t;

typedef struct Fire_Device{
	int active;
	int period; // number of hours
	int remaining_ticks; // hours left to historify

	date_t date;
} fire_device_t;

int relay_write(rl_t *, int);
int relay_read(rl_t *);

int digital_read(di_t *);

#endif
