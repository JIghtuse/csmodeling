/* file: tqueue.c */
#include "tasks.h"

struct q {
	task *tsk;
	struct q *prev;
	struct q *next; 
};

typedef struct q queue;
queue *qtail, *qhead;

queue *queue_add_zwtask (task *ts);
queue *queue_get_first (void);
queue *queue_get_small (void);
void destroy_queue (void);
