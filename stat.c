/* file: stat.c */
#include <stdio.h>
#include <math.h>
#include "stat.h"

void init_stat(stat *var) {
	var->sum = 0;
	var->sqsum = 0;
	var->count = 0;
}

void change_stat(stat *var, float val) {
	var->sum += val;
	var->sqsum += val * val;
	var->count++;
}

void print_stat(stat *var) {
	float avg = var->sum / var->count;
	float variance = var->sqsum / var->count - avg * avg;

	printf("Avg: %10.3f\t\tSqrt(Vrnce): %.6f\n",
			avg, sqrt(variance));
}
