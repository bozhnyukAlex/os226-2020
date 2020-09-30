#include "sched.h"

#define POOL_SIZE 16

struct task {
	void (*entry)(void *ctx);
	void *ctx;
	int priority;
	int deadline;
	// ...
};



static struct task taskpool[POOL_SIZE];
static int taskpool_n;
static enum policy policy;


void sched_new(void (*entrypoint)(void *aspace),
		void *aspace,
		int priority,
		int deadline) {
	struct task *t = &taskpool[taskpool_n++];
	t->entry = entrypoint;
	t->ctx = aspace;
	t->priority = priority;
	t->deadline = deadline;
	// ...
}

void sched_cont(void (*entrypoint)(void *aspace),
		void *aspace,
		int timeout) {
	// ...
}

void sched_time_elapsed(unsigned amount) {
	// ...
}

void sched_set_policy(enum policy _policy) {
	policy = _policy;
	// ...
}

int any_task_can_be_entried() {
	for (int i = 0; i < taskpool_n; i++) {
		if (*((int*)taskpool[i].ctx) >= 0) {
			return 1;
		}
	}
	return 0;
}

void sched_run(void) {
	while (any_task_can_be_entried()) {
		switch (policy) {
			case POLICY_FIFO:
				for (int i = 0; i < taskpool_n; i++) {
					if (*((int*)taskpool[i].ctx) >= 0) {
						taskpool[i].entry(taskpool[i].ctx);
					}
				}
				break;
			case POLICY_PRIO:

				break;

			case POLICY_DEADLINE: 

				break;
			default:
				printf("Wrong policy!");
				break;
		}
	}
	
}
