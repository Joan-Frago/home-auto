#ifndef TCP_SERVER_H

#define TCP_SERVER_H

#include <libxml2/libxml/parser.h>

#define MESSAGE_SIZE 4096

void *start_tcp_server(void*);
int talk(int *);

typedef struct Request{
	char *function;
	xmlNode *data;
}req_t;

int process_recv(char *recv_buf, char *resp_buf);

int read_request(req_t *, char *);
int call_target_function(req_t *, char *resp_buf);

#endif
