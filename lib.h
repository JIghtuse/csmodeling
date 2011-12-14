/* file: lib.h */

#define NAMELEN 128
#define TRACE
#undef TRACE
struct evt {
	char name[NAMELEN];
	float time;
	void *params;
	struct evt *next;
	struct evt *prev;
	void (*proc)(void);
};

typedef struct evt event;

float mt;
event *current;

void cancel(char *ev, float t);
void simulate(void);
void schedule(event *ev, float t);
void setproc(event *ev, void (*proc)(void));
void setpars(event *ev, void *params);
void setname(event *ev, char *name);
void clear_cal(void);
