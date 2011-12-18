/* file: model.c */
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include "lib.h"
#include "model.h"
#include "conf.h"
#include "stat.h"

#define EVAL 1000

unsigned int ql, Nb, Vb, mode;
float eps = 0.05, eval_time;
stat avtime, avtimeab;

int main(int argc, char *argv[]) {
	srand(time(NULL));
	
 	read_conf();
	printf("\n\n\tFirst modeling\n");

	for (Tk = 0.1; Tk <= 100; Tk += 5) {
		mode = SMALL_FIRST;
 		define_simtime(0);
		mode = SMALL_FIRST;
		run_simulating(eval_time);
		printf("Tk: %.2f\t", Tk);
		print_stat(&avtimeab);
	}
	
	read_conf();
	printf("\n\n\tSecond modeling\n");
	
	for (V = 32; V <= 1024; V += 32) {
		mode = FIFO;
		define_simtime(1);
		run_simulating(eval_time);
		printf("V: %5d\t", V);
		print_stat(&avtime);
	}

	read_conf();
	printf("\n\n\tThird modeling\n");
	for (N = 8; N <= 256; N += 16) {
		mode = FIFO;
		define_simtime(1);
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
	switch (mode) {
		case ABS_PRIORITY: case SMALL_FIRST:
			avg = avtimeab.sum / avtimeab.count;
			variance = sqrt(avtimeab.sqsum / avtimeab.count - avg * avg);	
			break;
		case FIFO:
			avg = avtime.sum / avtime.count;
			variance = sqrt(avtime.sqsum / avtime.count - avg * avg);
			break;
		default:
			break;
	}
	eval_time *= variance * variance / (avg * avg * eps * eps);
/* 	eval_time = (variance == 0) ? EVAL : eval_time;*/
 	eval_time = (eval_time > 2000000) ? EVAL : eval_time;
	printf("Simtime: %12.3f\t", eval_time);
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
	tsk->arrtime = mt;
	return tsk;
}

void arrive (void) {
	event *arr = (event *) malloc (sizeof(event));
	event *nextev = (event *) malloc (sizeof(event));
	task *ntask = newtask();

	setproc(arr, arrive);
	setname(arr, "Arrival");
	schedule(arr, mt + exp_rand(d, lambda));

	if ((ntask->nkern <= N - Nb) && (ntask->nmem <= V - Vb) && mode != ABS_PRIORITY) {
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

void start_compute (void) {
	event *nextev = (event *)malloc(sizeof(event));
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
		q = (mode == SMALL_FIRST) ? queue_get_small() : queue_get_first();
		setproc(nextevt, start_compute);
		setpars(nextevt, (void *)q);
		setname(nextevt, "Computing");
		schedule(nextevt, mt);
	}
}

void free_mem (void) {
	task *tsk = (task *) current->params;
	Vb -= tsk->nmem;
	if (ql > 0) {
		event *nextev = (event *)malloc(sizeof(event));
		queue *q;
		if (mode == SMALL_FIRST || mode == ABS_PRIORITY) {
			check_tasks_time();
		}
		q = (mode == SMALL_FIRST) ? queue_get_small() : queue_get_first();
		setproc(nextev, start_compute);
		setpars(nextev, (void *)q);
		setname(nextev, "Computing");
		schedule(nextev, mt);
	}
	free(tsk);
	tsk = NULL;
}

queue *queue_add_zwtask (task *ts)		/* zero waiting task */
{
	queue *q = (queue *)malloc(sizeof(queue));
	q->tsk = ts;
	ql++;
	ql--;
	return q;
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

queue *queue_get_first (void)
{
	queue *q;
	if (!qhead) return NULL;
	if ((qhead->tsk->nkern <= (N - Nb)) && (qhead->tsk->nmem <= (V - Vb))) {
		q = qhead;
		if (qhead->next) {
			qhead = qhead->next;
		} else {
			qhead = qtail = NULL;
		}
		ql--;
		return q;
	}
	return NULL;
}

queue *queue_get_small (void)
{
	queue *q = qhead, *tmp;
	if (!q) return NULL;
	if ((q == qtail) && (q->tsk->nkern <= (N - Nb)) && (q->tsk->nmem <= (V - Vb))) {
		qhead = qtail = NULL;
		ql--;
		return q;
	}
	while (q) {
		if ((q->tsk->nkern <= (N - Nb)) && (q->tsk->nmem <= (V - Vb))) {
			tmp = q->prev;
			if (tmp && q != qhead) tmp->next = q->next;
			if (q == qhead && qhead->next) {
				qhead = qhead->next;
				qhead->prev = NULL;
			}
			tmp = q->next;
			if (tmp && q != qtail) tmp->prev = q->prev;
			if (q == qtail && qtail->prev) {
				qtail = qtail->prev;
				qtail->next = NULL;
			}
			ql--;
			return q;
		}
		q = q->next;
	}
	return NULL;
}

void destroy_queue(void)
{
	queue *q = qhead, *tmp;
	if (!qhead) return;
	while (q) {
		free(q->tsk);
		q->tsk = NULL;
		tmp = q;
		q = q->next;
		free(tmp);
		tmp = NULL;
	}
	qhead = qtail = NULL;
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
