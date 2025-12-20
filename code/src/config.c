#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../inc/config.h"

char *UNIPI_SYS_BASE_DIR;

int load_config(void){
	printf("Loading config...\n");

	struct Var *var = alloc_var_mem();
	if(var == NULL){
		printf("Could not allocate memory for config variables\n");
		return -1;
	}

	UNIPI_SYS_BASE_DIR = (char *)malloc(sizeof(char[MAX_VAR_SIZE]));
	if(UNIPI_SYS_BASE_DIR == NULL){
		printf("Could not allocate memory for UNIPI_SYS_BASE_DIR (%ld bytes)\n",sizeof(char[MAX_VAR_SIZE]));

		free(var->name);
		free(var->value);
		free(var);
		return -1;
	}
	
	FILE *fp;
	fp = fopen("conf/home.conf", "r");
	if(fp == NULL){
		perror("Error opening config file\n");

		free(var->name);
		free(var->value);
		free(var);

		return -1;
	}

	char buf[MAX_VAR_SIZE];
	int buf_idx = 0;

	char c;
	while((c = getc(fp)) != EOF && buf_idx < MAX_VAR_SIZE){
		//printf("Read char: %c\n",c);
		if(c != '\n' && c != EOF){
			if(c != '='){
				buf[buf_idx] = c;
				buf_idx++;
			}else if(c == '='){
				buf[buf_idx] = '\0';
				buf_idx = 0;
				set_var_name(var, buf);
				strcpy(buf,"");
			}
		}else{
			// We have read the \n
			buf[buf_idx] = '\0';
			buf_idx = 0;
			set_var_value(var, buf);
			set_var(var);
		}
	}

	printf("Finished setting config variables.\n\n");

	fclose(fp);
	free(var->name);
	free(var->value);
	free(var);
	
	return 0;
}

int set_var_name(struct Var *var, char *buf){
	strcpy(var->name,buf);

	return 0;
}

int set_var_value(struct Var *var, char *buf){
	strcpy(var->value,buf);
	
	return 0;
}

int set_var(struct Var *var){
	if(strcmp(var->name,"UNIPI_SYS_BASE_DIR") == 0){
		strcpy(UNIPI_SYS_BASE_DIR,var->value);
	}


	printf("+ Variable set: %s = %s\n",var->name,var->value);

	return 0;
}

struct Var *alloc_var_mem(void){
	int nbytes = 0;

	struct Var *var = (struct Var *)malloc(sizeof(struct Var));
	if(var == NULL){
		printf("Could not allocate memory for struct Var (%ld bytes)\n",sizeof(struct Var));
		return NULL;
	}else{
		nbytes += sizeof(struct Var);
	}
	
	var->name = (char *)malloc(sizeof(char[MAX_VAR_SIZE]));
	var->value = (char *)malloc(sizeof(char[MAX_VAR_SIZE]));
	if(var->name == NULL || var->value == NULL){
		printf("Could not allocate memory for var->name or var->value (%ld bytes or %ld bytes)\n",sizeof(char[MAX_VAR_SIZE]),sizeof(char[MAX_VAR_SIZE]));
		return NULL;
	}else{
		nbytes += sizeof(char[MAX_VAR_SIZE]) * 2;
	}

	return var;
}
