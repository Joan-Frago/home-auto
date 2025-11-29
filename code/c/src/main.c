#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include "../inc/tcp_server.h"
#include "../inc/unipi_control.h"
#include "../inc/config.h"

int set_devices(void);

int main(){
	if(load_config() == -1){
		perror("load_config returned -1. Error loading configuration\n");
		return -1;
	}

	printf("Application PID is %ld\n\n",(long)getpid());

	// Set all devices before running (read from db and init Devices (Relay or Digital Input or Both))
	set_devices();

	// Start a new thread for each functionality
	pthread_t core_thread;
	pthread_create(&core_thread, NULL, core, NULL);

	pthread_t tcp_server_thread;
	pthread_create(&tcp_server_thread, NULL, start_tcp_server, NULL);

	pthread_join(core_thread, NULL);
	pthread_join(tcp_server_thread, NULL);

	return 0;
}

int set_devices(){
	// Set all devices before running
	// Read from db and init Devices
	// Each Device:
	// 		1 Relay
	// 		1 Digital Input
	// 		1 Relay and 1 Digital Input

	return 0;
}

void *start_tcp_server(void* arg){
	printf("Server Thread ID is %lu\n",(unsigned long)pthread_self());

	int sockfd, new_sockfd;
	struct sockaddr_in server_addr;
	struct sockaddr_in remote_addr;
	socklen_t addrlen;

	// Socket creation
	sockfd = socket(PF_INET, SOCK_STREAM, 0);
	if(sockfd < 0){
		perror("Error creating the socket\n");
		return NULL;
	}

	// Bind the socket
	server_addr.sin_family = PF_INET;
	server_addr.sin_port = htons(8080);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	int ret = bind(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
	if(ret < 0){
		perror("Error creating the socket\n");
		return NULL;
	}

	// Listen for connections
	ret = listen(sockfd,5);
	if(ret < 0){
		perror("Error listening for connections\n");
		return NULL;
	}

	addrlen = sizeof(remote_addr);

	char *address = inet_ntoa(server_addr.sin_addr);
	int port = ntohs(server_addr.sin_port);

	printf("Server listening on address %s port %d\n",address,port);

	for(;;){
		new_sockfd = accept(sockfd, (struct sockaddr *)&remote_addr, &addrlen);
		if(new_sockfd < 0){
			perror("Error accepting a connection\n");
			continue;
		}

		printf("New client connected\n");

		while(1){
			// Receive a message
			char recv_buf[MESSAGE_SIZE + 1];
			ssize_t bytes_recv = recv(new_sockfd, recv_buf, MESSAGE_SIZE, 0);
			if(bytes_recv == 0){
				// Client closed connection (EOF)
				printf("Client closed connection\n");
				break;
			}else if(bytes_recv < 0){
				perror("Error receiving data\n");
				break;
			}


			// Build a response message
			recv_buf[bytes_recv] = '\0'; // Null-terminate the received data

			char *base_msg = "Received data: ";
			size_t resp_len = strlen(base_msg) + bytes_recv + 1;
			char resp_buf[resp_len];

			strcpy(resp_buf, base_msg);
			strncat(resp_buf, recv_buf, MESSAGE_SIZE);
			printf("%s",resp_buf);

			// Send a response
			send(new_sockfd, resp_buf, strlen(resp_buf), 0);
		}

		close(new_sockfd);
		printf("Connection closed and ready for new client\n");
	}

	// Close the socket
	close(sockfd);
	return 0;
}

void *core(void* arg){
	printf("Core Thread ID is %lu\n\n",(unsigned long)pthread_self());

	// Relay
	struct Relay rl;
	rl.id_pin = "RO2.1";

	relay_write(&rl,1);

	// Digital Input
	struct DigitalInput di;
	di.id_pin = "DI1.4";
	di.pin = "1.4";

	int *di_status = (int *)malloc(sizeof(int));
	digital_read(&di, di_status);
	free(di_status);

	return 0;
}
