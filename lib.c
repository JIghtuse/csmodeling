/* file: lib.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib.h"

void setproc(event *ev, void (*proc)(void)) {
	ev->proc = proc;
	return;
}

void setname(event *ev, char *name) {
	strncpy(ev->name, name, NAMELEN);
	return;
}

void setpars(event *ev, void *pars) {
	ev->params = pars;
	return;
}

void schedule(event *ev, float t) {
	event *i, *j;
	ev->time = t;
	if (!current) {				/* there is no events yet*/
		current = ev;
		ev->next = ev->prev = NULL;
		return;
	}
	if (ev->time < current->time) {
		printf("Error scheduling event on past time\n");
		return;
	}
	i = current;	
	while ((i->time < ev->time) && i->next) {
		i = i->next;
	}
	if (i->time > ev->time) {	/* paste event on previous position */
		j = i->prev;
		i->prev = ev;
		ev->next = i;
		ev->prev = j;
		if (j) {
			j->next = ev;
		}
	} else {					/* paste event on next position */
		j = i->next;
		i->next = ev;
		ev->prev = i;
		ev->next = j;
		if (j) {
			j->prev = ev;
		}
	}
	return;
}

void simulate(void) {
	mt = 0;
	do {
#ifdef TRACE
		printf("Time: %.2f; Event: '%s'\n", current->time, current->name);
#endif
		mt = current->time;
		current->proc();
		current = current->next;
		free(current->prev);
	} while (strcmp(current->name, "EndSimulate"));
	clear_cal();
	return;
}

void clear_cal (void) {
	cancel("Arrival", 0);
	cancel("Computing", 0);
	cancel("Freeing memory", 0);
	cancel("Adding to queue", 0);
	cancel("End computing; return kernels", 0);
	mt = 0;
	current = NULL;
}

void cancel(char *name, float t) {
	event *ev, *i;
	if (!current || !current->next) {
#ifdef TRACE
		printf("Error cancelling\n");
#endif
		return;
	}
	ev = current->next;
	while (ev->next) {
		if (((ev->time) >= t) && (!strcmp(name, ev->name))) {
			i = ev->prev;
			i->next = ev->next;
			i = ev->next;
			i->prev = ev->prev;
			free(ev);
			ev = i;
		} else {
			ev = ev->next;
		}
	}
	if ((!strcmp(name, ev->name)) && ((ev->time) >= t)) {
		i = ev->prev;
		i->next = ev->next;
		free(ev);
	}
	return;
}

