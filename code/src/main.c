#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include "../inc/config.h"
#include "../inc/tcp_server.h"
#include "../inc/unipi_control.h"

void exit_handler();

struct Device devices[MAX_DEVICES];

int main(){
	signal(SIGINT, exit_handler);
	signal(SIGTERM, exit_handler);

	printf("Application PID is %ld\n\n",(long)getpid());

	if(load_config() == -1){
		perror("load_config returned -1. Error loading configuration\n");
		return -1;
	}

	// Set all devices before running (read from db and init Devices (Relay or Digital Input or Both))
	if(set_devices(devices) == -1){
		printf("Error: set_devices function returned with error code -1\n");
		return -1;
	}

	// Start a new thread for each functionality
	pthread_t core_thread;
	pthread_create(&core_thread, NULL, core, NULL);

	pthread_t tcp_server_thread;
	pthread_create(&tcp_server_thread, NULL, start_tcp_server, NULL);

	pthread_join(core_thread, NULL);
	pthread_join(tcp_server_thread, NULL);

	return 0;
}

void exit_handler(){
	printf("\nProgram terminated\n");
	printf("Exiting application...\n");

	exit(0);
}

/*
 * Infinite loop handling each Device's properties.
 * E.g. Historify and Date scheduling.
 * Only Devices must be used.
 * A single Relay or DigitalInput must not be used
 * since it has no logic attached to it.
 */
void *core(void* arg){
	printf("Core Thread ID is %lu\n",(unsigned long)pthread_self());

	// iterate through devices array and perform whatever actions have to be done

	return 0;
}
