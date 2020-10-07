#include "sched.h"
#include <stdlib.h>
#include "mylist.h"

#define POOL_SIZE 16
#define PRIOR_RANGE_MAX 11

struct task {
	void (*entry)(void *ctx);
	void *ctx;
	int priority;
	int deadline;
	int index;
	int ready_time; 
};



int fifo_cmp(struct task *t1, struct task *t2);
int prior_cmp(struct task *t1, struct task *t2);
int index_cmp(struct task *t1, struct task *t2);
int deadline_cmp(struct task *t1, struct task *t2);
void policy_push(struct task *value);


static int (*policy_cmp)(struct task *t1, struct task *t2);
static struct task taskpool[POOL_SIZE];
static int taskpool_n;
static enum policy policy;
int prior_cnt[PRIOR_RANGE_MAX];
int zero_deadline_cnt;
int timeout_was_set;
int global_time;
struct List run_queue;
struct List wait_queue;

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
	for (int i = 0; i < taskpool_n; i++) {
		if (aspace == taskpool[i].ctx) {
			taskpool[i].ready_time = global_time + timeout;
			if (timeout == 0) {
				policy_push(&taskpool[i]);
			}
			else if (timeout > 0) {
				push(&wait_queue, &taskpool[i]);
			}
		}
	}
}



void sched_time_elapsed(unsigned amount) {
	global_time += amount;
	struct Node *curr_w = wait_queue.head;
	while (curr_w) { // анализируем wait_queue
		if (((struct task*)curr_w->data)->ready_time <= global_time) {
			policy_push(curr_w->data);
			struct Node* temp = curr_w;
			curr_w = curr_w->next;
			deleteNodeByValue(&wait_queue, temp->data);
		}
		else {
			curr_w = curr_w->next;
		}
	}
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
	run_queue = createList();
	for (int i = 0; i < taskpool_n; i++) {
		push(&run_queue, &taskpool[i]);
	}
	if (policy == POLICY_DEADLINE) {
		for (int i = 0; i < zero_deadline_cnt; i++) {
			push(&run_queue, run_queue.head->data);
			deleteHead(&run_queue);
		}
	}

	struct Node* curr = run_queue.head;
	while (curr) {
		((struct task*) curr->data)->entry(((struct task*) curr->data)->ctx);
		deleteHead(&run_queue);
		curr = run_queue.head;
	}
}

void policy_push(struct task *value) {
	switch (policy) {
		case POLICY_FIFO: {
			push(&run_queue, value);
			break;
		}
		case POLICY_PRIO: {
			int val_prior_cnt = prior_cnt[value->priority];
			if (val_prior_cnt == 1) { // вставляем в начало
				insertToBegin(&run_queue, createNode(value));
			}
			else if (val_prior_cnt > 1) { // вставляем в конец round-robin-отрезка
				struct Node* curr = run_queue.head;
				
				if (((struct task*)run_queue.end->data)->priority == value->priority) {
					push(&run_queue, value);
				}
				else {
					while (value->priority != ((struct task*)curr->data)->priority && curr) {
						curr = curr->next;
					}
					struct Node* before_end;
					struct Node* to_push = createNode(value);
					while (value->priority == ((struct task*)curr->data)->priority && curr) {
						if (((struct task*)curr->next)->priority != value->priority) {
							before_end = curr;
						}
						curr = curr->next;
					}
					before_end->next = to_push;
					to_push->next = curr;
				}

			}
			break;
		}
		case POLICY_DEADLINE: {
			int val_deadline = value->deadline;
			int this_deadline_count = 0;
			for (int i = 0; i < taskpool_n; i++) {
				if (taskpool[i].deadline == val_deadline) {
					this_deadline_count++;
				}
			}
			if (this_deadline_count == 1) { // вставим в начало
				insertToBegin(&run_queue, createNode(value));
			}
			else if (this_deadline_count > 1) { // вот тут по приоритетам...
				int val_prior_cnt = prior_cnt[value->priority];
				struct Node* curr = run_queue.head;
			    while (((struct task*)curr->data)->deadline != val_deadline && curr) { // ищем, откуда в списке начинается отрезок с нашим дедлайном
					curr = curr->next;
				}
				// Теперь curr на начале отрезка с нашим дедлайном.
				int dead_prior_cnt = 0;
				for (int i = 0; i < taskpool_n; i++) {
					if (taskpool[i].deadline == val_deadline && taskpool[i].priority == value->priority) {
						dead_prior_cnt++;
					}
				}
				if (dead_prior_cnt == 1) { // как в приоритетах - один такой - вставили в начало
					if (curr == run_queue.head) {
						insertToBegin(&run_queue, createNode(value));
					}
					else {
						struct Node* prev_curr = run_queue.head;
						while (prev_curr->next != curr) {
							prev_curr = prev_curr->next;
						}
						insertAfterEl(&run_queue, indexOf(&run_queue, prev_curr), value);
					}
				}
				else if (dead_prior_cnt > 1) { // тут round-robin, curr на начале отрезка с дедлайном
				    if (((struct task*)run_queue.end->data)->priority == value->priority) { // если конец отрезка совпал с концом списка - просто вставка
						push(&run_queue, value);
					}
					else {
						while (value->priority != ((struct task*)curr->data)->priority && curr) { // ищем начало отрезка с приоритетом
							curr = curr->next;
						}
						struct Node* before_end;
						struct Node* to_push = createNode(value);
						while (value->priority == ((struct task*)curr->data)->priority && curr) { //ищем конец отрезка с приоритетом и пред-конец
							if (((struct task*)curr->next->data)->priority != value->priority) {
								before_end = curr;
							}
							curr = curr->next;
						}
						before_end->next = to_push;
						to_push->next = curr;
					}
				}
			}
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