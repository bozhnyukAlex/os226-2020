#define _GNU_SOURCE

#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

#include "timer.h"

static struct timeval initv;

int timer_cnt(void) {
	struct itimerval curr;
	getitimer(ITIMER_REAL, &curr); 
	return (initv.tv_sec - curr.it_value.tv_sec) * 1000 + (initv.tv_usec - curr.it_value.tv_usec) / 1000;
}

extern void timer_init_period(int ms, hnd_t hnd) {
	initv.tv_sec  = ms / 1000;
	initv.tv_usec = ms * 1000;

	const struct itimerval setup_it = {
		.it_value    = initv,
		.it_interval = initv,
	};

	if (-1 == setitimer(ITIMER_REAL, &setup_it, NULL)) {
		perror("setitimer");
	}

	struct sigaction act = {
		.sa_sigaction = hnd,
		.sa_flags = SA_RESTART,
	};
	sigemptyset(&act.sa_mask);

	if (-1 == sigaction(SIGALRM, &act, NULL)) {
		perror("signal set failed");
		exit(1);
	}
}
