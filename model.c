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
float eps = 0.0000000005, eval_time;
stat avtime, avtimeab, avstat;

int main(int argc, char *argv[]) {
	srand(time(NULL));
	
	read_conf();
	printf("\n\n\tFirst modeling\n");

	for (Tk = 0.1; Tk <= 5; Tk += 0.2) {
		mode = SMALL_FIRST;
 		define_simtime(0);
		mode = SMALL_FIRST;
		run_simulating(eval_time);
		printf("Tk: %.2f\t", Tk);
		print_stat(&avstat);
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
		case 0:
			avg = avstat.sum / avstat.count;
			variance = sqrt(avstat.sqsum / avstat.count - avg * avg);	
/*			variance = sqrt(avstat.sqsum / avstat.count - avg * avg);	*/
			break;
		case 1:
			avg = avtime.sum / avtime.count;
			variance = sqrt(avtime.sqsum / avtime.count - avg * avg);
			break;
		default:
			break;
	}
	eval_time *= ((variance / avg) / eps) * ((variance / avg) / eps);
	eval_time = (variance == 0) ? EVAL : eval_time;
	eval_time = (eval_time > 2000000) ? EVAL : eval_time;
	printf("Simtime: %12.3f\t", eval_time);
	return;
}

void run_simulating (float simtime)
{
	event *firstev = (event *) malloc(sizeof(event));
	event *lastev = (event *) malloc(sizeof(event));
	ql = Nb = Vb = 0;
	
	if (mode == FIFO) {
		init_stat(&avtime);
	} else {
		init_stat(&avtimeab);
		init_stat(&avstat);
	}
	setproc(firstev, arrive);
	setname(firstev, "Arrival");
	schedule(firstev, mt);

	setproc(lastev, finish);
	setname(lastev, "EndSimulate");
	schedule(lastev, simtime);

	simulate();
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

	if ((ntask->nkern <= N - Nb) && (ntask->nmem <= V - Vb) && ql == 0) {
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
		change_stat(&avtimeab, mt - ts->arrtime);
		change_stat(&avstat, ts->comptime);
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
	if (!ql) {
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
	queue *qelem;
	if (!qhead) {
		return NULL;
	}
	if (ql && (qhead->tsk->nkern <= (N - Nb)) && (qhead->tsk->nmem <= (V - Vb))) {
		qelem = qhead;
		qhead = qhead->next;
		ql--;
		return qelem;
	}
	return NULL;
}

queue *queue_get_small (void) /* DESTROY */
{
	queue *q, *tmp;
	if ((q = qhead) != NULL) {
		do {
			if ((q->tsk->nkern <= (N - Nb)) && (q->tsk->nmem <= (V - Vb))) {
				tmp = q->prev;
				if (tmp) tmp->next = q->next;
				tmp = q->next;
				if (tmp) tmp->prev = q->prev;
				ql--;
				return q;
			}
			q = q->next;
		} while (q && q->prev && q->prev != qtail);
	}
	return NULL;
}

void destroy_queue(void)
{
	queue *q;
	if (!ql) return;
	q = qhead;
	do {
		free(q->tsk);
		q->tsk = NULL;
		q = q->next;
		if (q->prev) {
			free (q->prev);
			q->prev = NULL;
		}
	} while (q->prev != qtail);
}

void check_tasks_time (void) /* DESTROY THIS! */
{
	queue *q, *tmp, *qn, *qp, *qhp;
	task *ts;
	if ((q = qhead) != NULL) {
		do {
			if (!q) return;
			ts = q->tsk;
			if ((mt - ts->arrtime) >= Tk) {
				mode = ABS_PRIORITY;

				qn = q->next;
				qp = q->prev;
				qhp = qhead->prev;

				tmp = qhead;
				qhead = qhead->prev;
				if (qhead) {
					qhead->next = q;
					qhead = qhead->next;
					qhead->prev = qhp;
					qhead->next = NULL;
				}

				if (qp) qp->next = tmp;
				if (qn) qn->prev = tmp;
				tmp->prev = qp;
				tmp->next = qn;
/* 				printf("Mode is ABS_PRIORITY");*/
				return;
			}
			q = q->next;
		} while (q && q != qtail);
	}
/* 	printf("Mode is SMALL_FIRST");*/
	mode = SMALL_FIRST;
}

void finish (void)
{
	destroy_queue();
}
