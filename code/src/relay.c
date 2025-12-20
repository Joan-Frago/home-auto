#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../inc/unipi_control.h"
#include "../inc/config.h"

int relay_write(struct Relay *relay, int new_state){
	int ret_val = 0;

	size_t path_len = strlen(UNIPI_SYS_BASE_DIR) + strlen(relay->id_pin) + strlen("/value") + 1;

	// Allocate memory for the file path
	char *file_path = (char *)malloc(path_len);
	if(file_path == NULL){
		perror("Could not allocate memory for file path");
		ret_val = -1;
	}
	else{
		strcpy(file_path, UNIPI_SYS_BASE_DIR);
		strcat(file_path, relay->id_pin);
		strcat(file_path, "/value");

		FILE *fp = fopen(file_path,"w");
		putc('0' + new_state, fp);
		putc(EOF,fp);
		fclose(fp);
	}
	free(file_path);
	
	return ret_val;
}
