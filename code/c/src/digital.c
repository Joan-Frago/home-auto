#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../inc/unipi_control.h"
#include "../inc/config.h"


int digital_read(struct DigitalInput *di, int *dest){
	int ret_val = 0;

	size_t path_len = strlen(UNIPI_SYS_BASE_DIR) + strlen(di->id_pin) + strlen("/value") + 1;

	// Allocate memory for the file path
	char *file_path = (char *)malloc(path_len);
	if(file_path == NULL){
		perror("Could not allocate memory for file path");
		ret_val = -1;
	}
	else{
		strcpy(file_path, UNIPI_SYS_BASE_DIR);
		strcat(file_path, di->id_pin);
		strcat(file_path, "/value");

		printf("%s\n",file_path);
		//FILE *fp = fopen(fileDir,"w");
	}
	
	free(file_path);

	*dest = 1;

	return ret_val;
}
