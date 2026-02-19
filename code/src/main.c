#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include "../inc/config.h"
#include "../inc/tcp_server.h"
#include "../inc/device.h"

static void *core(void*);
static void exit_handler(int);

static int historify_device(device_t *device);
static int fire_device(device_t *device);

int main(){
	signal(SIGINT, exit_handler);
	signal(SIGTERM, exit_handler);

	printf("Application PID is %ld\n\n",(long)getpid());

	if(load_config() == -1){
		perror("load_config returned -1. Error loading configuration\n");
		return -1;
	}

	// Set all devices before running (read from xml file and init Devices (Relay or Digital Input or Both))
	if(set_devices() == -1){
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

static void exit_handler(int signal){
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
 *
 * Iterate through devices array and perform whatever
 * actions have to be done.
 */
static void *core(void* arg){
	printf("Core Thread ID is %lu\n",(unsigned long)pthread_self());

	device_t *devices = get_devices_arr();

	for(;;){
		int i;
		for(i=0; i<MAX_DEVICES; i++){
			historify_device(&devices[i]);
			fire_device(&devices[i]);
		}
		sleep(1);
	}

	return 0;
}

static int historify_device(device_t *device){
	if(device->hist.active == 1){
		if(device->hist.period == 0){
			// TODO: Historify all changes
			// e.g. A relay output state change
			// e.g. A digital input state change
		}
		else{
			if(device->hist.remaining_ticks == 1){
				// TODO: Historify
				
				// RELAY
				if(device->rl.id_pin){
					int rl_val = relay_read(&device->rl);
					if(rl_val == -1)
						return -1;

					printf("Historify device \"%s\" Relay \"%s\". Value = %d.\n",device->name, device->rl.id_pin, rl_val);
				}

				// DIGITAL INPUT
				if(device->di.id_pin){
					int di_val = digital_read(&device->di);
					if(di_val == -1)
						return -1;

					printf("Historify device \"%s\" DigitalInput \"%s\". Value = %d.\n",device->name, device->di.id_pin, di_val);
				}

				device->hist.remaining_ticks = device->hist.period;
			}
			else{
				device->hist.remaining_ticks--;
			}
		}
	}
	return 0;
}

static int fire_device(device_t *device){
	if(device->fire.active == 1){
		if(device->fire.period == 0){
			// TODO: Use date
		}
		else{
			// TODO: Use period
			if(device->fire.remaining_ticks == 1){
				// Fire relay
				if(device->rl.id_pin){
					if(relay_write(&device->rl, 1) == -1){

					}

					printf("Device \"%s\" with Relay \"%s\" fired.\n",device->name, device->rl.id_pin);
				}

				device->fire.remaining_ticks = device->fire.period;
			}
			else{
				device->fire.remaining_ticks--;
			}
		}
	}
	return 0;
}
