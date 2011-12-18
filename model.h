/* file: model.h */
#define KSI ((float)rand()/RAND_MAX)
#define SMALL_FIRST 0
#define ABS_PRIORITY 1
#define FIFO 2


typedef struct {
	unsigned int nkern;
	unsigned int nmem;
	float comptime;
	float arrtime;
} task;

struct q {
	task *tsk;
	struct q *prev;
	struct q *next; 
};

typedef struct q queue;
queue *qtail, *qhead;

task *newtask(void);

void arrive (void);
void queue_add_task (void);
void start_compute (void);
void free_kernels (void);
void free_mem (void);
void finish (void);

void check_tasks_time (void);
void swap_tasks (queue *q1, queue *q2);
float unif_rand(float n);
float exp_rand(float d, float param);

queue *queue_add_zwtask (task *ts);
queue *queue_get_first (void);
queue *queue_get_small (void);
void destroy_queue (void);

void run_simulating(float simtime);
void define_simtime(int mode);	
