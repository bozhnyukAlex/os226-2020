#include "sched.h"
#include <stdlib.h>
#include "mylist.h"

#define POOL_SIZE 16
#define PRIOR_RANGE_MAX 11
#define READY_TO_EXECUTE 0


int fifo_cmp(struct task *t1, struct task *t2);
int prior_cmp(struct task *t1, struct task *t2);
int index_cmp(struct task *t1, struct task *t2);
int deadline_cmp(struct task *t1, struct task *t2);

int can_be_entried(int i);
int any_task_can_be_entried(int start_closed, int end_unclosed);


static int (*policy_cmp)(struct task *t1, struct task *t2);
static struct task taskpool[POOL_SIZE];
static int taskpool_n;
static enum policy policy;
int prior_cnt[PRIOR_RANGE_MAX];
int zero_deadline_cnt;
int timeout_was_set;
struct List list;
struct task *curWait;
int global_time;
struct List queue;


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
	t->ready_time = global_time;
	if (deadline <= 0) {
		deadline = 0;
		zero_deadline_cnt++;
	}
	t->deadline = deadline;
	prior_cnt[priority]++;
	
	
}

void sched_cont(void (*entrypoint)(void *aspace),
		void *aspace,
		int timeout) {

		
	
}



void sched_time_elapsed(unsigned amount) {
	global_time += amount;

}

void sched_set_policy(enum policy _policy) {
	policy = _policy;
	switch (policy) {
		case POLICY_FIFO:
			policy_cmp = fifo_cmp;
			break;
		case POLICY_PRIO:
			policy_cmp = prior_cmp;
			break;
		case POLICY_DEADLINE: 
			policy_cmp = deadline_cmp;
			break;
		default:
			printf("Unknown policy\n");
			abort();
			break;
	}
}


void sched_run(void) {
	qsort(taskpool, taskpool_n, sizeof(struct task), policy_cmp);
	queue = createList();
	for (int i = 0; i < taskpool_n; i++) {
		push(&queue, &taskpool[i]);
	}
	
	
}











int fifo_cmp(struct task *t1, struct task *t2){
	return -1;
}

int prior_cmp(struct task *t1, struct task *t2){
	int delta = t2->priority - t1->priority;
	if (delta) {
		return delta;
	}
	return index_cmp(t1, t2);
}

int index_cmp(struct task *t1, struct task *t2) {
	return t1->index - t2->index;
}

int deadline_cmp(struct task *t1, struct task *t2) {
	int delta = t1->deadline - t2->deadline;
	if (delta) {
		return delta;
	}
	return prior_cmp(t1, t2);
}

//deprecated
int can_be_entried(int i) {
	return *((int*)taskpool[i].ctx) >= 0; // access to ctx->cnt
}

//deprecated
int any_count_more_zero(int start_closed, int end_unclosed) {
	for(int i = start_closed; i < end_unclosed; i++) {
		if (taskpool[i].counter >= 0) {
			return 1;
		}
	}
	return 0;
}

//deprecated
int any_task_can_be_entried(int start_closed, int end_unclosed) {
	for (int i = start_closed; i < end_unclosed; i++) {
		if (can_be_entried(i)) { 
			return 1;
		}
	}
	return 0;
}