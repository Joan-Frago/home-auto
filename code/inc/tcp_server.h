#include <libxml2/libxml/parser.h>

#define MESSAGE_SIZE 1000
#define FUNCTION_NAME_SIZE 50
#define MAX_VARS_PER_REQUEST 200

void *start_tcp_server(void*);
int talk(int *);

struct Request{
	char function[FUNCTION_NAME_SIZE];
	xmlNode *data;
};

int process_recv(char *buf, int buf_len);
