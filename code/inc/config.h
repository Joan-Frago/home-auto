#define MAX_VAR_SIZE 256

extern char *UNIPI_SYS_BASE_DIR;

struct Var{
	char *name;
	char *value;
};

int load_config(void);
int set_var_name(struct Var *, char *);
int set_var_value(struct Var *, char *);
int set_var(struct Var *);
struct Var *alloc_var_mem(void);
