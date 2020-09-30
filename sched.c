#include "sched.h"
#include <stdlib.h>

#define POOL_SIZE 16
#define PRIOR_RANGE_MAX 11

struct task {
	void (*entry)(void *ctx);
	void *ctx;
	int priority;
	int deadline;
	int index;
};



static struct task taskpool[POOL_SIZE];
static int taskpool_n;
static enum policy policy;
int prior_cnt[PRIOR_RANGE_MAX];


void sched_new(void (*entrypoint)(void *aspace),
		void *aspace,
		int priority,
		int deadline) {
	struct task *t = &taskpool[taskpool_n];
	t->index = taskpool_n;
	taskpool_n++;
	t->entry = entrypoint;
	t->ctx = aspace;
	t->priority = priority;
	t->deadline = deadline;
	prior_cnt[priority]++;
	
}

void sched_cont(void (*entrypoint)(void *aspace),
		void *aspace,
		int timeout) {
	// ...
}

int prior_cmp(struct task *t1, struct task *t2){
	return t2->priority - t1->priority;
}

int index_cmp(struct task *t1, struct task *t2) {
	return t1->index - t2->index;
}

void sched_time_elapsed(unsigned amount) {
	// ...
}

void sched_set_policy(enum policy _policy) {
	policy = _policy;
	// ...
}

int task_cnt_more_zero(int i) {
	return *((int*)taskpool[i].ctx) >= 0; // access to ctx->cnt
}

int any_task_can_be_entried(int start_closed, int end_unclosed) {
	for (int i = start_closed; i < end_unclosed; i++) {
		if (task_cnt_more_zero(i)) { 
			return 1;
		}
	}
	return 0;
}

void exec_fifo(int start_closed, int end_unclosed) {
	while(any_task_can_be_entried(start_closed, end_unclosed)) {
		for (int i = start_closed; i < end_unclosed; i++) {
			if (task_cnt_more_zero(i)) {
				//sched_cont(taskpool[i].entry, taskpool[i].ctx, 0);
				taskpool[i].entry(taskpool[i].ctx);
			}
		}
	}
}

void exec_prio() {
	qsort(taskpool, taskpool_n, sizeof(struct task), prior_cmp);
	for (int i = 0; i < taskpool_n; i++) {
		int cur_priority = taskpool[i].priority;
		int cur_task_prior_cnt = prior_cnt[cur_priority];

		if (cur_task_prior_cnt == 1) {
			while (task_cnt_more_zero(i)) {
				taskpool[i].entry(taskpool[i].ctx);
			}
		}
		else if (cur_task_prior_cnt > 1) {
			exec_fifo(i, i + cur_task_prior_cnt);
			i += (cur_task_prior_cnt - 1);
		}
	}
}





void sched_run(void) {
	switch (policy){
		case POLICY_FIFO:
			exec_fifo(0, taskpool_n);
			break;
		
		case POLICY_PRIO:
			exec_prio();
			break;

		case POLICY_DEADLINE:
			break;
		default:
			break;
	}
	
}
