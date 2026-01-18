#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../inc/config.h"

var_t vars[MAX_VARS];

int load_config(void){
	printf("Loading config...\n");

	FILE *fp;
	fp = fopen(CONFIG_FILE_DIR, "r");
	if(fp == NULL){
		perror("Error opening config file\n");

		return -1;
	}

	char buf[MAX_VAR_SIZE];
	int buf_idx, var_idx = 0;

	int c;
	while((c = getc(fp)) != EOF && buf_idx < MAX_VAR_SIZE - 1){
		//printf("Read char: %c\n",c);
		if(c != '\n' && c != EOF){
			if(c != '='){
				buf[buf_idx] = c;
				buf_idx++;
			}else if(c == '='){
				buf[buf_idx] = '\0';
				buf_idx = 0;
				//printf("%s\n",buf);
				set_var_name(&vars[var_idx], buf);
				buf[0] = '\0';
			}
		}else{
			// We have read the \n
			buf[buf_idx] = '\0';
			buf_idx = 0;
			//printf("%s\n",buf);
			set_var_value(&vars[var_idx], buf);

			printf("+ Variable set: %s = %s\n",vars[var_idx].name,vars[var_idx].value);
			var_idx++;
		}
	}

	printf("Finished setting config variables.\n\n");

	fclose(fp);
	
	return 0;
}

int set_var_name(var_t *var, char *buf){
	strcpy(var->name,buf);

	return 0;
}

int set_var_value(var_t *var, char *buf){
	strcpy(var->value,buf);
	
	return 0;
}

char *get_var_value(char *var_name){
	for(int i=0; i<MAX_VARS; i++){
		if(strcmp(vars[i].name, var_name)==0){
			return vars[i].value;
		}
	}
	return NULL;
}
