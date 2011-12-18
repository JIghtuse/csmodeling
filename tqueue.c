#include <stdlib.h>
#include "conf.h"
#include "tqueue.h"

queue *queue_add_zwtask (task *ts)		/* zero waiting task */
{
	queue *q = (queue *)malloc(sizeof(queue));
	q->tsk = ts;
	ql++;
	ql--;
	return q;
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
