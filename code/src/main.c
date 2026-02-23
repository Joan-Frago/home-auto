#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include "../inc/config.h"
#include "../inc/tcp_server.h"
#include "../inc/device.h"
#include "../inc/logger.h"
#include "../inc/loggerconf.h"
#include "../inc/historify.h"

#define LOG_FIRE

#define LOG_FILE "log/home.log"

static void *core(void*);
static void exit_handler(int);

static int fire_device(device_t *device);

int main(){
	signal(SIGINT, exit_handler);
	signal(SIGTERM, exit_handler);

    logger_initConsoleLogger(NULL);
    logger_initFileLogger(LOG_FILE, 1024 * 1024, 5);
    logger_setLevel(LogLevel_DEBUG);

	LOG_INFO("Application PID is %ld",(long)getpid());

	if(load_config() == -1){
		LOG_ERROR("load_config returned -1. Error loading configuration");
		return -1;
	}

	// Set all devices before running (read from xml file and init Devices (Relay or Digital Input or Both))
	if(set_devices() == -1){
		LOG_ERROR("Error: set_devices function returned with error code -1");
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
	LOG_INFO("Program terminated");
	LOG_INFO("Exiting application...");

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

					#ifdef LOG_FIRE
					LOG_DEBUG("Device \"%s\" with Relay \"%s\" fired.",device->name, device->rl.id_pin);
					#endif
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
