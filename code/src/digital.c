#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../inc/unipi_control.h"
#include "../inc/config.h"

int digital_read(di_t *di){
	char *base_dir = get_var_value(UNIPI_SYS_BASE_DIR);
	size_t path_len = strlen(base_dir) + strlen(di->id_pin) + strlen("/value") + 1;

	// Allocate memory for the file path
	char *file_path = (char *)malloc(path_len);
	if(file_path == NULL){
		perror("Could not allocate memory for file path");
		return -1;
	}

	strcpy(file_path, base_dir);
	strcat(file_path, di->id_pin);
	strcat(file_path, "/value");

	//printf("%s\n",file_path);
	FILE *fp = fopen(file_path,"r");
	if(fp == NULL) return -1;

	int status;
	fscanf(fp,"%d",&status);

	fclose(fp);

	free(file_path);

	return status;
}
