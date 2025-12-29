#include "../inc/util.h"

int char2int(char *a){
	if(*a >= '0' && *a <= '9') return *a - '0';
	else return -1;
}
