#define KSI ((float)rand()/RAND_MAX)

typedef struct {
	unsigned int nkern;
	unsigned int nmem;
	float comptime;
	float arrtime;
} task;


task *newtask(void);

float unif_rand(float n);
float exp_rand(float d, float param);

