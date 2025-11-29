#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "../inc/tcp_server.h"
#include "../inc/unipi_control.h"
#include "../inc/config.h"

int foo();

int main(){
	if(load_config() == -1){
		perror("load_config returned -1. Error loading configuration\n");
		return -1;
	}

	return start_tcp_server();
}

int start_tcp_server(){
	int sockfd, new_sockfd;
	struct sockaddr_in server_addr;
	struct sockaddr_in remote_addr;
	socklen_t addrlen;

	// Socket creation
	sockfd = socket(PF_INET, SOCK_STREAM, 0);
	if(sockfd < 0){
		perror("Error creating the socket\n");
		return -1;
	}

	// Bind the socket
	server_addr.sin_family = PF_INET;
	server_addr.sin_port = htons(8080);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	int ret = bind(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
	if(ret < 0){
		perror("Error creating the socket\n");
		return -1;
	}

	// Listen for connections
	ret = listen(sockfd,5);
	if(ret < 0){
		perror("Error listening for connections\n");
		return -1;
	}

	addrlen = sizeof(remote_addr);

	char *address = inet_ntoa(server_addr.sin_addr);
	int port = ntohs(server_addr.sin_port);

	// Console messages
	printf("Server listening on address %s port %d\n",address,port);
	printf("Server PID is %ld\n\n",(long)getpid());

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

			// Business logic
			foo();

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

int foo(){
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
