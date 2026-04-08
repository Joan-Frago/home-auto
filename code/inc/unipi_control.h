#ifndef UNIPI_CONTROL_H
#define UNIPI_CONTROL_H

#include <stdint.h>

#define XML_XPATH_EXPR_SIZE 255

typedef enum MB_CONNECTION_TYPE {
	TCP,
	RS485
} mb_con_t;

#define REGISTER_COUNT 11

typedef struct {
	char correction_op; // The operation to make ( *, /, +, - )
	char *name;
	char *symbol;
	char *line;
	uint8_t correction_no; // The number to apply to the operation ( 1, 10, 20, 100, ... )
	float value;
	float last_value;
	int id;
} reg_t;

typedef struct {
	int slave;
	int tcp_port;
	char *tcp_addr;
	mb_con_t connection_type;
	reg_t registers[REGISTER_COUNT];
} mb_t;

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

#define DATE_ENTRIES 5

typedef struct {
	char *date;
	int running;

	int period;
	int period_countdown;

	int duration;
	int duration_countdown;

	int start;
} entry_t;

typedef struct Date{
	entry_t entries[DATE_ENTRIES];
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

void analyzer_set_registers(reg_t registers[REGISTER_COUNT]);
float modbus_read(mb_t, reg_t);

#endif
