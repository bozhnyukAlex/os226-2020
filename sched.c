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
int was_tick;
int timeout_was_set;
struct List list;
struct task *curWait;


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
	t->timer = READY_TO_EXECUTE;
	if (deadline <= 0) {
		deadline = 0;
		zero_deadline_cnt++;
	}
	t->deadline = deadline;
	t->counter = *((int*)t->ctx);
	prior_cnt[priority]++;
	
	
}

void sched_cont(void (*entrypoint)(void *aspace),
		void *aspace,
		int timeout) {
	if (timeout > 0) {
		timeout_was_set++;
	}
	for (int i = 0; i < taskpool_n; i++) {
		if (taskpool[i].ctx == aspace) {
			taskpool[i].timer = timeout;
			curWait = &taskpool[i];
		}
	}
}



void sched_time_elapsed(unsigned amount) {
	for (int i = 0; i < taskpool_n; i++) {
		if (taskpool[i].timer != READY_TO_EXECUTE) {
			taskpool[i].timer--;
		}
	}
	was_tick++;
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

void exec_fifo(int start_closed, int end_unclosed) {
	while(any_task_can_be_entried(start_closed, end_unclosed)) {
		for (int i = start_closed; i < end_unclosed; i++) {
			if (can_be_entried(i)) {
				taskpool[i].entry(taskpool[i].ctx);
			}
		}
	}
}

void exec_prio_alg(int start_closed, int end_unclosed) {
	for (int i = start_closed; i < end_unclosed; i++) {
		int cur_priority = taskpool[i].priority;
		int cur_task_prior_cnt = prior_cnt[cur_priority];

		if (cur_task_prior_cnt == 1) {
			while (can_be_entried(i)) {
				taskpool[i].entry(taskpool[i].ctx);
			}
		}
		else if (cur_task_prior_cnt > 1) {
			qsort(&taskpool[i], cur_task_prior_cnt, sizeof(struct task), index_cmp);
			exec_fifo(i, i + cur_task_prior_cnt);
			i += (cur_task_prior_cnt - 1);
		}
	}
} 

void exec_prio() {
	qsort(taskpool, taskpool_n, sizeof(struct task), prior_cmp);
	exec_prio_alg(0, taskpool_n);
}

void exec_deadline() {
	qsort(taskpool, taskpool_n, sizeof(struct task), deadline_cmp);
	for (int i = zero_deadline_cnt; i < taskpool_n; i++) {
		int dead_cnt = 0, 
			j = i;
		int cur_deadline = taskpool[i].deadline;
		while (taskpool[j].deadline == cur_deadline) {
			dead_cnt++;
			j++;
		}
		if (dead_cnt == 1) {
			while (can_be_entried(i)) {
				taskpool[i].entry(taskpool[i].ctx);
			}
		} 
		else if (dead_cnt > 1) {
			qsort(&taskpool[i], dead_cnt, sizeof(struct task), prior_cmp);
			exec_prio_alg(i, i + dead_cnt);
			i += (dead_cnt - 1);
		}
	}
	qsort(taskpool, zero_deadline_cnt, sizeof(struct task), prior_cmp);
	exec_prio_alg(0, zero_deadline_cnt);
}

void round_robin_push(int start_closed, int end_unclosed) {
	while (any_count_more_zero(start_closed, end_unclosed)) {
		for (int i = start_closed; i < end_unclosed; i++) {
			if (taskpool[i].counter >= 0) {
				push(&list, &taskpool[i]);
				taskpool[i].counter--;
			}
		}
	}
}



void sched_run(void) {
	qsort(taskpool, taskpool_n, sizeof(struct task), policy_cmp);
	list = createList();

	switch (policy) {
		case POLICY_FIFO: {
			round_robin_push(0, taskpool_n);
			break;
		}
	}

	while (list.head) {
		list.head->data->entry(list.head->data->ctx);
		deleteHead(&list);
		struct Node* cur = list.head;
		if (timeout_was_set) {
			while (cur->data != curWait) {
				cur = cur->next;
			}
			while (cur->data == curWait) {
				shiftRight(&list, indexOf(&list, cur), cur->data->timer);
				cur = cur->next;
			}
			timeout_was_set = 0;
		}
		
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


int can_be_entried(int i) {
	return *((int*)taskpool[i].ctx) >= 0; // access to ctx->cnt
}

int any_count_more_zero(int start_closed, int end_unclosed) {
	for(int i = start_closed; i < end_unclosed; i++) {
		if (taskpool[i].counter >= 0) {
			return 1;
		}
	}
	return 0;
}

int any_task_can_be_entried(int start_closed, int end_unclosed) {
	for (int i = start_closed; i < end_unclosed; i++) {
		if (can_be_entried(i)) { 
			return 1;
		}
	}
	return 0;
}