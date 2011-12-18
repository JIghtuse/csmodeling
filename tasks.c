#include <math.h>
#include <stdlib.h>
#include "tasks.h"
#include "conf.h"


float exp_rand (float d, float param) {
	return d - log(KSI) / param ;
}

float unif_rand (float n) {
	return KSI * (n - 1) + 1;
}

task *newtask(void) {
	task *tsk = (task *) malloc (sizeof(task));
	tsk->nkern = unif_rand(kmax);
	tsk->nmem = unif_rand(vmax);
	tsk->comptime = exp_rand(D, mu);
	return tsk;
}
