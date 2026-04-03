#include "../inc/device.h"
#include "../inc/logger.h"
#include <string.h>
#include <time.h>
#include <stdlib.h>

static void fire_device_date(device_t *);
static void fire_device_period(device_t *);
static entry_t *fire_get_matching_entry(device_t *);

int fire_device(device_t *device){
	if(device->fire.active == 0) return 0;

	if(device->fire.period == 0)
		fire_device_date(device);
	else
		fire_device_period(device);

	return 0;
}

static void fire_device_date(device_t *device){
	entry_t *entry = fire_get_matching_entry(device);
	if(entry == NULL) return;
	
	if(entry->running){
		if(entry->period_countdown == 0){
			entry->period_countdown = entry->period;
			entry->duration_countdown = entry->duration;
		}

		if(entry->duration_countdown > 0){
			// TODO: Power Relay
			LOG_INFO("Powering relay...");
			relay_write(&device->rl, 1);

			entry->duration_countdown--;
		}

		entry->period_countdown--;
	}
	else {
		time_t now = time(NULL);
		struct tm *t = localtime(&now);
		
		if(entry->start ==  t->tm_hour){
			entry->running = 1;
			entry->period_countdown = 0; // so it starts right away

			LOG_INFO("Starting date entry...");
		}
	}
}

static void fire_device_period(device_t *device){
	if(device->fire.remaining_ticks == 1){
		// Fire relay
		if(device->has_rl == 1){
			if(relay_write(&device->rl, 1) == -1){
				LOG_ERROR("Can\'t write to relay %s", device->rl.id_pin);
				return;
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

static entry_t *fire_get_matching_entry(device_t *device){
	entry_t *entry = device->fire.date.entries;
	entry_t *last_entry = NULL;

	time_t now = time(NULL);
	struct tm *t = localtime(&now);

	int current_mmdd = (t->tm_mon + 1) * 100 + t->tm_mday; // e.g. 403 for 3rd April
	int max_past_mmdd = -1;

	int i = 0;
	while(i<DATE_ENTRIES){
		if(entry->date == NULL)
			goto next;

		if(strlen(entry->date) != 5 || entry->date[2] != '/'){
			LOG_ERROR("Unsupported date format. The format must be dd/mm");
			goto next;
		}

		int day, month;

		if(sscanf(entry->date, "%2d/%2d", &day, &month) != 2)
			goto next;

		int entry_mmdd = month * 100 + day;

		if(entry_mmdd <= current_mmdd){
			if(entry_mmdd > max_past_mmdd){
				max_past_mmdd = entry_mmdd;
				last_entry = entry;
			}
		}

		next:
			i++;
			entry++;
	}

	// If no valid entry can be found (last_entry = NULL),
	// maybe the user simply doesn't want to do anything.
	return last_entry;
}
