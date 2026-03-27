#ifndef TCP_SERVER_H

#define TCP_SERVER_H

#include <libxml/parser.h>

#define MESSAGE_SIZE 4096

void *start_tcp_server(void*);

typedef struct Request{
	char *function;
	xmlNode *data;
}req_t;

#endif
