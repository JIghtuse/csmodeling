/* file: conf.c */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "conf.h"

int read_conf(void) { 
	const int buf = 128;
	float par_value;
	char *str = (char *) malloc(sizeof(char) * buf);
	char *par_name = (char *) malloc(sizeof(char) * (buf - 12));
	FILE *fp = fopen(CONF, "rt");
	while (fgets(str, buf, fp)) {
		sscanf(str, "%s = %f", par_name, &par_value);
		parse_param(par_name, par_value);
	}
	free(str);
	free(par_name);
	fclose(fp);
	return 0;
}

void parse_param (char *par_name, float par_value) {
	if (!strcmp(par_name, "N")) {
		N = par_value;
	} else if (!strcmp(par_name, "V")) {
		V = par_value;
	} else if (!strcmp(par_name, "d")) {
		d = par_value;
	} else if (!strcmp(par_name, "lambda")) {
		lambda = par_value;
	} else if (!strcmp(par_name, "D")) {
		D = par_value;
	} else if (!strcmp(par_name, "mu")) {
		mu = par_value;
	} else if (!strcmp(par_name, "vmax")) {
		vmax = par_value;
	} else if (!strcmp(par_name, "delta")) {
		delta = par_value;
	} else if (!strcmp(par_name, "kmax")) {
		kmax = par_value;
	} else if (!strcmp(par_name, "Tk")) {
		Tk = par_value;
	} else {
		printf("Unexpected value in config file!\n");
	}
}
