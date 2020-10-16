#define _GNU_SOURCE

#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#include "timer.h"
#include "sched.h"
#include "ctx.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*a))

/* AMD64 Sys V ABI, 3.2.2 The Stack Frame:
The 128-byte area beyond the location pointed to by %rsp is considered to
be reserved and shall not be modified by signal or interrupt handlers */
#define SYSV_REDST_SZ 128

extern void tramptramp(void);

static void bottom(void);

struct task {
	void (*entry)(void *as);
	void *as;
	int priority;

	struct ctx ctx;
	char stack[8192];

	// timeout support
	int waketime;

	// policy support
	struct task *next;
	int id;
};

static volatile int time;
static int tick_period;

static struct task *current;
static int current_start;
static struct task *runq;
static struct task *waitq;

static struct task idle;
static struct task taskpool[16];
static int taskpool_n;

static sigset_t irqs;

static void irq_disable(void) {
	sigprocmask(SIG_BLOCK, &irqs, NULL);
}

static void irq_enable(void) {
	sigprocmask(SIG_UNBLOCK, &irqs, NULL);
}

static int prio_cmp(struct task *t1, struct task *t2) {
	return t1->priority - t2->priority;
}

static void policy_run(struct task *t) {
	struct task **c = &runq;

	while (*c && prio_cmp(*c, t) <= 0) {
		c = &(*c)->next;
	}
	t->next = *c;
	*c = t;
}

static void wait_push(struct task *t) {
	struct task **c = &waitq;
	while (*c && (*c)->waketime < t->waketime) {
		c = &(*c)->next;
	}
	t->next = *c;
	*c = t;
}

static void hctx_push(greg_t *regs, unsigned long val) { // положить значение на стек
	regs[REG_RSP] -= sizeof(unsigned long);
	*(unsigned long *) regs[REG_RSP] = val;
}

static void top(int sig, siginfo_t *info, void *ctx) {
	ucontext_t *uc = (ucontext_t *) ctx;
	greg_t *regs = uc->uc_mcontext.gregs;

	unsigned long oldsp = regs[REG_RSP];
	regs[REG_RSP] -= SYSV_REDST_SZ;
	hctx_push(regs, regs[REG_RIP]);
	hctx_push(regs, sig);
	hctx_push(regs, regs[REG_RBP]);
	hctx_push(regs, oldsp);
	hctx_push(regs, (unsigned long) bottom);
	regs[REG_RIP] = (greg_t) tramptramp; 
}

int sched_gettime(void) {
	int cnt1 = timer_cnt();
	int time1 = time;
	int cnt2 = timer_cnt();
	int time2 = time;

	return (cnt1 <= cnt2) ? time1 + cnt2 : time2 + cnt2;
}


static struct task* updateTask() {
	struct task *pr = current;
	current = runq;
	runq = runq->next;
	return pr;
}
static void switch_ctx(struct task *old, struct task *new) {
	current_start = sched_gettime();
	ctx_switch(&old->ctx, &new->ctx);
}

// FIXME below this line
// включает текущую задачу
static void tasktramp(void) {
	irq_enable(); // во время работы задачи надо реагировать на таймер
	current->entry(current->as);
	struct task *pr = updateTask();
	switch_ctx(pr, current);
}


void sched_new(void (*entrypoint)(void *aspace),
		void *aspace,
		int priority) {
	
	if (ARRAY_SIZE(taskpool) <= taskpool_n) {
		fprintf(stderr, "No mem for new task\n");
		return;
	}
	struct task *t = &taskpool[taskpool_n];
	t->id = taskpool_n;

	taskpool_n++;
	t->entry = entrypoint;
	t->as = aspace;
	t->priority = priority;

	ctx_make(&t->ctx, tasktramp, t->stack, sizeof(t->stack));

	irq_disable(); // при работе с очередью нельзя давать прерываниям таймера мешать
	policy_run(t);
	irq_enable();
}


static void bottom(void) {
	irq_disable();
	time += tick_period;

	while (waitq != NULL && waitq->waketime <= sched_gettime()) {
		struct task *temp = waitq;
		policy_run(temp);
		waitq = waitq->next;
	} 

	if (sched_gettime() - current_start >= tick_period) { // когда квант времени текущей задачи кончился - вытесняем задачу
		debug(runq);
		policy_run(current);
		struct task *pr = updateTask();
		switch_ctx(pr, current);
	//	printf("@%d\nin bottom: ", sched_gettime());
		

	}

	irq_enable();

	
}

void sched_sleep(unsigned ms) {
	if (ms == 0) { // как sched_cont(0), просто вставка в очередь
		irq_disable();
		policy_run(current);
		struct task *pr = updateTask();
		switch_ctx(pr, current);
		irq_enable();
		return;
	}

	current->waketime = sched_gettime() + ms;

	irq_disable();
	wait_push(current);
	struct task *pr = updateTask();
	switch_ctx(pr, current);
	irq_enable();
}
void debug(struct task *t) {
	return;
	if (t == NULL) {
		printf("NULL\n");
		return;
	}
	printf("%d -> ", t->id);
	debug(t->next);
}


void sched_run(int period_ms) {
	sigemptyset(&irqs);
	sigaddset(&irqs, SIGALRM);// сигнал того, что кончился таймер, добавлен

	tick_period = period_ms;
	timer_init_period(period_ms, top);

	sigset_t none;
	sigemptyset(&none);

	irq_disable();
	idle.id = -2;

	current = &idle;
	while (runq != NULL || waitq != NULL) {
		if (runq != NULL) {
			policy_run(current);
			struct task *pr = updateTask();
			switch_ctx(pr, current);
		//	printf("!!!!%d\nin run: ", sched_gettime());
			debug(runq);
		}
		else { // если остались только ожидающие, то ждем таймер, чтобы отправить в обработчике ожидающих на исполнение
			sigsuspend(&none);
		}
	}
}