
#define _GNU_SOURCE

#include <stdio.h>
#include <signal.h>
#include <ucontext.h>
#include <sys/ucontext.h>
#include <stdint.h>

#include "syscall.h"
#include "util.h"

extern void init(void);
extern int sys_print(char *str, int len);

unsigned f(unsigned val, int h, int l) {
        return (val & ((1ul << (h + 1)) - 1)) >> l;
}       

int enc2reg(unsigned enc) { 
        switch(enc) {
        case 0: {
			printf("REG_RAX\n");
			return REG_RAX;
		}
        case 1:{
			printf("REG_RCX\n"); 
			return REG_RCX;
		}
        case 2:{ 
			printf("REG_RDX\n");
			return REG_RDX;
		}
        case 3: { 
			printf("REG_RBX\n");
			return REG_RBX;
		}
        case 4: { 
			printf("REG_RSP\n");
			return REG_RSP;
		}
        case 5: {
			printf("REG_RBP\n");
			return REG_RBP;
		}
        case 6: {
			printf("REG_RSI\n");
			return REG_RSI;
		}
        case 7: {
			printf("REG_RDI\n");
			return REG_RDI;
		}
        default: break;
        }
		printf("abort0\n");
        abort();
}
/*
void sighnd(int sig, siginfo_t *info, void *ctx) {
        ucontext_t *uc = (ucontext_t *) ctx;
        greg_t *regs = uc->uc_mcontext.gregs;

		static int n_calls;

        uint8_t *ins = (uint8_t *)regs[REG_RIP];
        if (ins[0] != 0x8b) {
                abort();
        }
		printf("ins before: ");
		for (int i = 0; i < ARRAY_SIZE(ins); i++) {
			printf("%x ", ins[i]);
		}
		printf("\n");

        uint8_t *next = &ins[2];
		printf("next: %p %x\n", next, *next);

        int dst = enc2reg(f(ins[1], 5, 3)); // REG_RAX
		printf("dst: %x\n", dst);

        int rm = f(ins[1], 3, 0);
        if (rm == 4) {
                abort();
        }
		printf("rm: %d\n", rm);
        int base = enc2reg(rm); // REG_RAX
		printf("base: %x\n", base);
        int off = 0;
        switch(f(ins[1], 7, 6)) {
        case 0:
				printf("case 0\n");
                break;
        case 1: 
				printf("case 1\n");
                off = *(int8_t*)next;
                next += 1;
                break;
        case 2:
				printf("case 2\n");
                off = *(uint32_t *)&next;
                next += 4;
                break;
        default:
				printf("case default\n");
                break;
        }
		printf("%p %x\n", next, *next);
        regs[dst] = 100 + (++n_calls);
        regs[REG_RIP] = (unsigned long)next;

		ins = (uint8_t *)regs[REG_RIP];
		printf("ins after: ");
		for (int i = 0; i < ARRAY_SIZE(ins); i++) {
			printf("%x ", ins[i]);
		}
		
} */

static void sighnd(int sig, siginfo_t *info, void *ctx) {
	ucontext_t *uc = (ucontext_t *) ctx;
	greg_t *regs = uc->uc_mcontext.gregs;
	
	unsigned long rax = regs[REG_RAX];

	uint8_t *ins = (uint8_t *)regs[REG_RIP];
	//printf("%x %x\n", ins[0], ins[1]);
	if (ins[0] != 0xCD) {
		abort();
	}
	uint8_t *next = &ins[2];
	if (rax != os_syscall_nr_print) {
		printf("abort3\n");
		abort();
	}

	char* rbx = regs[REG_RBX];
	unsigned long rcx = regs[REG_RCX];

	sys_print(rbx, rcx);
	regs[REG_RIP] = (unsigned long)next;
}

int main(int argc, char *argv[]) {
	struct sigaction act = {
		.sa_sigaction = sighnd,
		.sa_flags = SA_RESTART,
	};
	sigemptyset(&act.sa_mask);

	if (-1 == sigaction(SIGSEGV, &act, NULL)) {
		perror("signal set failed");
		return 1;
	}

	init();
	return 0;
}
