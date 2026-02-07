#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../inc/unipi_control.h"
#include "../inc/config.h"

int relay_write(rl_t *relay, int new_state){
	char *base_dir = get_var_value(UNIPI_SYS_BASE_DIR);
	size_t path_len = strlen(base_dir) + strlen(relay->id_pin) + strlen("/value") + 1;

	// Allocate memory for the file path
	char *file_path = (char *)malloc(path_len);
	if(file_path == NULL){
		perror("Could not allocate memory for file path");
		return -1;
	}

	strcpy(file_path, base_dir);
	strcat(file_path, relay->id_pin);
	strcat(file_path, "/value");

	FILE *fp = fopen(file_path,"w");
	if(fp == NULL){
		printf("Could not open file \"%s\"\n",file_path);
		return -1;
	}

	fprintf(fp,"%d",new_state);
	//putc('0' + new_state, fp);
	//putc(EOF,fp);
	fclose(fp);

	free(file_path);
	
	return 0;
}

int relay_read(rl_t *rl){
	char *base_dir = get_var_value(UNIPI_SYS_BASE_DIR);
	size_t path_len = strlen(base_dir) + strlen(rl->id_pin) + strlen("/value") + 1;

	char *file_path = (char *)malloc(path_len);
	if(file_path == NULL){
		perror("Could not allocate memory for file path\n");
		return -1;
	}

	strcpy(file_path, base_dir);
	strcat(file_path, rl->id_pin);
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
