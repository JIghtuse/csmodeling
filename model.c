/* file: model.c */
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include "lib.h"
#include "model.h"
#include "conf.h"
#include "stat.h"
#include "tqueue.h"

#define EVAL 1000

unsigned int mode;
const float eps = 0.05;
float eval_time;

stat avtime, avtimeab;

int main(int argc, char *argv[]) {
 	srand(time(NULL));
	
 	read_conf();
	printf("\n\n\tFirst modeling\n");

	for (Tk = 0.1; Tk <= 200; Tk += 10) {
		mode = SMALL_FIRST;
 		define_simtime(SMALL_FIRST);
		mode = SMALL_FIRST;
		run_simulating(eval_time);
		printf("Tk: %.2f\t", Tk);
		print_stat(&avtimeab);
	}
	
	read_conf();
	printf("\n\n\tSecond modeling\n");
	
	for (V = 32; V <= 1024; V += 32) {
		mode = FIFO;
		define_simtime(FIFO);
		run_simulating(eval_time);
		printf("V: %5d\t", V);
		print_stat(&avtime);
	}

	read_conf();
	printf("\n\n\tThird modeling\n");
	for (N = 8; N <= 256; N += 8) {
		mode = FIFO;
		define_simtime(FIFO);
		run_simulating(eval_time);
		printf("N: %5d\t", N);
		print_stat(&avtime);
	}

	return 0;
}

void define_simtime(int mode)
{
	float avg, variance;
	eval_time = EVAL;
	run_simulating(eval_time);
	if (mode == ABS_PRIORITY || mode == SMALL_FIRST) {
		avg = avtimeab.sum / avtimeab.count;
		variance = avtimeab.sqsum / avtimeab.count - avg * avg;	
	} else if (mode == FIFO) {
		avg = avtime.sum / avtime.count;
		variance = avtime.sqsum / avtime.count - avg * avg;
	}


	eval_time *= variance * variance / (avg * avg * eps * eps);
 	eval_time = (variance == 0) ? EVAL : eval_time;
 	eval_time = (eval_time > 2000000) ? EVAL : eval_time;
	return;
}

void run_simulating (float simtime)
{
	event *firstev = (event *) malloc(sizeof(event));
	event *lastev = (event *) malloc(sizeof(event));
	ql = Nb = Vb = 0;
	mt = 0;
	
	if (mode == FIFO) {
		init_stat(&avtime);
	} else {
		init_stat(&avtimeab);
	}
	setproc(firstev, arrive);
	setname(firstev, "Arrival");
	schedule(firstev, mt);

	setproc(lastev, finish);
	setname(lastev, "EndSimulate");
	schedule(lastev, simtime);

	simulate();
	destroy_queue();
}


void arrive (void) {
	event *arr = (event *) malloc (sizeof(event));
	event *nextev = (event *) malloc (sizeof(event));
	task *ntask = newtask();
	ntask->arrtime = mt;

	setproc(arr, arrive);
	setname(arr, "Arrival");
	schedule(arr, mt + exp_rand(d, lambda));

	if ((ntask->nkern <= N - Nb) && (ntask->nmem <= V - Vb)
			&& mode != ABS_PRIORITY) {
		queue *q = queue_add_zwtask(ntask);
		setproc(nextev, start_compute);
		setname(nextev, "Computing");
		setpars(nextev, (void *)q);
		schedule(nextev, mt);
	} else {
		setproc(nextev, queue_add_task);
		setname(nextev, "Adding to queue");
		setpars(nextev, (void *)ntask);
		schedule(nextev, mt);
	}
}

void queue_add_task (void)
{
	queue *q = (queue *)malloc(sizeof(queue));
	task *ts = (task *)current->params;
	q->tsk = ts;
	if (!qhead) {
		q->next = q->prev = NULL;
		qtail = qhead = q;
	} else {
		q->next = NULL;
		qtail->next = q;
		q->prev = qtail;
		qtail = q;
	}
	ql++;
}

void start_compute (void) {
	event *nextev;
	queue *q = (queue *)current->params;
	task *ts;
	if (q == NULL) {
		return;
	}
	ts = q->tsk;
	free(q);
	q = NULL;
	Nb += ts->nkern;
	Vb += ts->nmem;
	if (mode == FIFO) {
		change_stat(&avtime, mt - ts->arrtime);
	} else {
		change_stat(&avtimeab, ts->comptime + mt - ts->arrtime);
	}
	nextev = (event *)malloc(sizeof(event));
	setproc(nextev, free_kernels);
	setname(nextev, "End computing; return kernels");
	setpars(nextev, (void *)ts);
	schedule(nextev, mt + ts->comptime);
}

void free_kernels (void) {
	event *nextev = (event *)malloc(sizeof(event));
	task *tsk = (task *) current->params;
	Nb -= tsk->nkern;
	setproc(nextev, free_mem);
	setpars(nextev, (void *)tsk);
	setname(nextev, "Freeing memory");
	schedule(nextev, mt + delta);
	if (ql > 0) {
		event *nextevt = (event *)malloc(sizeof(event));
		queue *q;
		if (mode == SMALL_FIRST || mode == ABS_PRIORITY) {
			check_tasks_time();
		}
		q = (mode == SMALL_FIRST) ?
			queue_get_small() : queue_get_first();
		setproc(nextevt, start_compute);
		setpars(nextevt, (void *)q);
		setname(nextevt, "Computing");
		schedule(nextevt, mt);
	}
}

void free_mem (void) {
	task *tsk = (task *) current->params;
	Vb -= tsk->nmem;
	free(tsk);
	tsk = NULL;
	if (ql > 0) {
		event *nextev = (event *)malloc(sizeof(event));
		queue *q;
		if (mode == SMALL_FIRST || mode == ABS_PRIORITY) {
			check_tasks_time();
		}
		q = (mode == SMALL_FIRST) ?
			queue_get_small() : queue_get_first();
		setproc(nextev, start_compute);
		setpars(nextev, (void *)q);
		setname(nextev, "Computing");
		schedule(nextev, mt);
	}
}

void check_tasks_time (void)
{
	if (!qhead) {
		mode = SMALL_FIRST;
		return;
	}
	if ((mt - qhead->tsk->arrtime) >= Tk) {
		mode = ABS_PRIORITY;
		return;
	}
	mode = SMALL_FIRST;
	return;
}

void finish (void)
{
	return;
}
