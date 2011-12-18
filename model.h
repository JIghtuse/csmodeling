/* file: model.h */
#define SMALL_FIRST 0
#define ABS_PRIORITY 1
#define FIFO 2


void arrive (void);
void queue_add_task (void);
void start_compute (void);
void free_kernels (void);
void free_mem (void);
void finish (void);

void check_tasks_time (void);


void run_simulating(float simtime);
void define_simtime(int mode);	
