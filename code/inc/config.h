#ifndef CONFIG_H

#define CONFIG_H

#define CONFIG_FILE_DIR "conf/home.conf"

#define MAX_VAR_SIZE 256
#define MAX_VARS 8

#define UNIPI_SYS_BASE_DIR	"UNIPI_SYS_BASE_DIR"
#define TCP_SERVER_PORT		"TCP_SERVER_PORT"
#define MODBUS_TCP_ADDR		"MODBUS_TCP_ADDR"
#define MODBUS_TCP_PORT		"MODBUS_TCP_PORT"
#define MODBUS_RTU_DEVICE	"MODBUS_RTU_DEVICE"
#define MODBUS_RTU_BAUD		"MODBUS_RTU_BAUD"
#define MODBUS_RTU_PARITY	"MODBUS_RTU_PARITY"
#define MODBUS_RTU_DATABIT	"MODBUS_RTU_DATABIT"
#define MODBUS_RTU_STOPBIT	"MODBUS_RTU_STOPBIT"

typedef struct Var{
	char name[MAX_VAR_SIZE];
	char value[MAX_VAR_SIZE];
}var_t;

extern var_t vars[MAX_VARS];

int load_config(void);
char *get_var_value(char *var_name);

#endif
