#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>

#include "../inc/tcp_server.h"
#include "../inc/device.h"
#include "../inc/config.h"
#include "../inc/logger.h"
#include "../inc/sb.h"

static int talk(int *);
static int process_recv(char *recv_buf, char *resp_buf);
static int read_request(req_t *, char *);
static int call_target_function(req_t *, char *resp_buf);
static int escape_buf(char *buf, int buf_len);
static int set_error(char *resp_buf, const char *err_msg);

void *start_tcp_server(void* arg){
	LOG_INFO("Server Thread ID is %lu",(unsigned long)pthread_self());

	int sockfd, new_sockfd;
	struct sockaddr_in server_addr;
	struct sockaddr_in remote_addr;
	socklen_t addrlen;

	// Socket creation
	sockfd = socket(PF_INET, SOCK_STREAM, 0);
	if(sockfd < 0){
		LOG_ERROR("Error creating the socket");
		return NULL;
	}

	// Bind the socket
	server_addr.sin_family = PF_INET;
	server_addr.sin_port = htons(atoi(get_var_value(TCP_SERVER_PORT)));
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	int opt = 1;
	int ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if(ret){
		LOG_ERROR("Error setting options to the socket");
		return NULL;
	}

	ret = bind(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
	if(ret < 0){
		LOG_ERROR("Error creating the socket");
		return NULL;
	}

	// Listen for connections
	ret = listen(sockfd,5);
	if(ret < 0){
		LOG_ERROR("Error listening for connections");
		return NULL;
	}

	addrlen = sizeof(remote_addr);

	char *address = inet_ntoa(server_addr.sin_addr);
	int port = ntohs(server_addr.sin_port);

	LOG_INFO("Server listening on address %s port %d",address,port);

	for(;;){
		new_sockfd = accept(sockfd, (struct sockaddr *)&remote_addr, &addrlen);
		if(new_sockfd < 0){
			LOG_ERROR("Error accepting a connection");
			continue;
		}

		// printf("New client connected\n");

		talk(&new_sockfd);

		close(new_sockfd);
		// printf("Ready for new client\n");
	}

	// Close the socket
	close(sockfd);
	return 0;
}

static int talk(int *sockfd){
	while(1){
		// Receive a message
		char recv_buf[MESSAGE_SIZE];
		memset(recv_buf, 0, strlen(recv_buf)); // Set the buffer to 0 aka "empty the string"

		ssize_t bytes_recv = recv(*sockfd, recv_buf, MESSAGE_SIZE-1, 0);
		if(bytes_recv == 0){
			// printf("Client closed connection\n");
			break;
		}else if(bytes_recv < 0){
			LOG_ERROR("Error receiving data");
			break;
		}

		recv_buf[bytes_recv] = '\0';

		// Buffer where the response will be stored
		char resp_buf[MESSAGE_SIZE];
		memset(resp_buf, 0, strlen(resp_buf)); // Set the buffer to 0 aka "empty the string"

		// Process received data
		if(process_recv(recv_buf, resp_buf) != 0){
			LOG_ERROR("Error: Could not process received data from client");
		}

		// Escape response buffer
		//escape_buf(resp_buf, strlen(resp_buf));

		// Send a response
		send(*sockfd, resp_buf, strlen(resp_buf), 0);
	}
	return 0;
}

/*
 * Process received data and call corresponding function.
 *
 * Data must be an xml.
 */
static int process_recv(char *recv_buf, char *resp_buf){
	// printf("Received buffer: %s\n", recv_buf);
	//strcat(resp_buf, "ok");

	req_t req;
	read_request(&req, recv_buf);

	// printf("Request function name: %s\n", req.function);
	if(req.data){
		// printf("Request data: %s\n", BAD_CAST req.data);
	}

	// Call another function that reads req.function and calls the target function
	if(call_target_function(&req, resp_buf) == -1){
		LOG_ERROR("Error: call_target_function returned -1");
		return -1;
	}

	return 0;
}

/*
 * Reads the function name and content from the request and sets the request struct
 */
static int read_request(req_t *req, char *xml_doc_str){
	xmlDoc *req_xml_doc = xmlReadDoc(BAD_CAST xml_doc_str, NULL, NULL, 0);
	if(req_xml_doc == NULL){
		LOG_ERROR("Error: Could not parse request xml.");
		return -1;
	}

	xmlXPathContext *xpath_ctx = xmlXPathNewContext(req_xml_doc);
	if(xpath_ctx == NULL){
		LOG_ERROR("Error: Unable to create new XPath context.");
		return -1;
	}

	xmlNode *root = xmlDocGetRootElement(req_xml_doc);

	xmlNode *device = root->children;
	device = device->next;

	xpath_ctx->node = device;

	// Request function
	xmlXPathObjectPtr xpath_obj = xmlXPathEvalExpression(BAD_CAST "/request/function", xpath_ctx);

	xmlNode *node = xpath_obj->nodesetval->nodeTab[0];
	xmlChar *content = xmlNodeGetContent(node);

	req->function = strdup((char *)content);

	xmlFreeNode(node);
	xmlFree(content);
	xmlXPathFreeObject(xpath_obj);

	// Request data
	xpath_obj = xmlXPathEvalExpression(BAD_CAST "/request/data", xpath_ctx);

	if(xpath_obj && !xmlXPathNodeSetIsEmpty(xpath_obj->nodesetval)){
		node = xpath_obj->nodesetval->nodeTab[0];

		req->data = node;
	}

	xmlXPathFreeObject(xpath_obj);

	return 0;
}

static int call_target_function(req_t *req, char *resp_buf){
	const char *err_msg;

	if(strcmp(req->function, "get_all_devices") == 0){
		if(get_all_devices(resp_buf) == -1){
			char *msg = "Error: get_all_devices returned -1";
			LOG_ERROR("%s", msg);
			err_msg = msg;

			goto error;
		}
	} else if(strcmp(req->function, "get_device_pin_status") == 0){
		if(get_device_pin_status(resp_buf, req->data) == -1){
			char *msg = "Error: get_device_pin_status returned -1";
			LOG_ERROR("%s", msg);
			err_msg = msg;

			goto error;
		}
	} else if(strcmp(req->function, "get_device") == 0){
		if(get_device(resp_buf, req->data) == -1){
			char *msg = "Error: get_device returned -1";
			LOG_ERROR("%s", msg);
			err_msg = msg;

			goto error;
		}
	} else if(strcmp(req->function, "update_pin_state") == 0){
		if(update_pin_state(resp_buf, req->data) == -1){
			char *msg = "Error: update_pin_state returned -1";
			LOG_ERROR("%s", msg);
			err_msg = msg;

			goto error;
		}
	} else {
		char *msg;
		sprintf(msg, "Request not found: \"%s\"", req->function);
		LOG_INFO(msg);
		err_msg = msg;

		goto error;
	}

	return 0;

error:
	set_error(resp_buf, err_msg);
	return -1;
}

static int set_error(char *resp_buf, const char *err_msg){
	StringBuilder *sb = sb_create();

	sb_append(sb, "<error>");
	sb_appendf(sb, "\t%s", err_msg);
	sb_append(sb, "</error>");


	char *temp = sb_concat(sb);
	if(temp){
		strncpy(resp_buf, temp, MESSAGE_SIZE);
		free(temp);
	}
	sb_free(sb);

	return 0;
}

static int escape_buf(char *buf, int buf_len){
	char tmp_buf[MESSAGE_SIZE];
	int idx_buf = 0;
	int idx_tmp_buf = 0;

	int c;
	while(idx_tmp_buf < MESSAGE_SIZE && (c = buf[idx_buf++])!='\0'){
		if(c == '\n' || c == '\t' || c == '\r' || c == '\"'){
			tmp_buf[idx_tmp_buf++] = '\\';
		}
		tmp_buf[idx_tmp_buf++] = c;
	}

	strcpy(buf, tmp_buf);

	return 0;
}
