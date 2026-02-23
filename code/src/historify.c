#include "../inc/historify.h"
#include "../inc/logger.h"
#include "../inc/unipi_control.h"
#include "../inc/device.h"

#define LOG_HISTORIFY

int historify_device(device_t *device){
	if(device->hist.active == 1){
		// update last value

		if(device->rl.id_pin){
			device->rl.last_value = device->rl.value;
			device->rl.value = relay_read(&device->rl);
		}

		if(device->di.id_pin){
			device->di.last_value = device->di.value;
			device->di.value = digital_read(&device->di);
		}

		if(device->hist.period == 0){
			// Historify all changes

			// RELAY
			if(device->rl.id_pin){
				if(device->rl.value != device->rl.last_value){

					#ifdef LOG_HISTORIFY
					LOG_DEBUG("Historify device \"%s\" Relay \"%s\". Value = %d.",device->name, device->rl.id_pin, device->rl.value);
					#endif
				}
			}

			// DIGITAL INPUT
			if(device->di.id_pin){
				if(device->di.value != device->di.last_value){
					#ifdef LOG_HISTORIFY
					LOG_DEBUG("Historify device \"%s\" DigitalInput \"%s\". Value = %d.",device->name, device->di.id_pin, device->di.value);
					#endif
				}
			}
		}
		else{
			if(device->hist.remaining_ticks == 1){
				
				// RELAY
				if(device->rl.id_pin){
					#ifdef LOG_HISTORIFY
					LOG_DEBUG("Historify device \"%s\" Relay \"%s\". Value = %d.",device->name, device->rl.id_pin, device->rl.value);
					#endif
				}

				// DIGITAL INPUT
				if(device->di.id_pin){
					#ifdef LOG_HISTORIFY
					LOG_DEBUG("Historify device \"%s\" DigitalInput \"%s\". Value = %d.",device->name, device->di.id_pin, device->di.value);
					#endif
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
