#define _GNU_SOURCE

#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

#include "timer.h"

struct timeval init_value;

int timer_cnt(void) {
	struct itimerval *curr;
	getitimer(ITIMER_REAL, curr);
	__time_t time_el_div = 1000 * (init_value.tv_sec - curr->it_value.tv_sec);
	suseconds_t time_el_mod = (init_value.tv_usec - curr->it_value.tv_usec) / 1000;
	return time_el_div + time_el_mod;
}

extern void timer_init_period(int ms, hnd_t hnd) {
	struct timeval initv = {
		.tv_sec  = ms / 1000,
		.tv_usec = ms * 1000,
	};

	init_value = initv;

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


