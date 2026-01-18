#ifndef CONFIG_H

#define CONFIG_H

#define CONFIG_FILE_DIR "conf/home.conf"

#define MAX_VAR_SIZE 256
#define MAX_VARS 8

#define UNIPI_SYS_BASE_DIR "UNIPI_SYS_BASE_DIR"
#define TCP_SERVER_PORT "TCP_SERVER_PORT"

typedef struct Var{
	char name[MAX_VAR_SIZE];
	char value[MAX_VAR_SIZE];
}var_t;

extern var_t vars[MAX_VARS];

int load_config(void);
int set_var_name(var_t *, char *);
int set_var_value(var_t *, char *);
char *get_var_value(char *var_name);

#endif
