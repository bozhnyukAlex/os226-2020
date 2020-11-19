
#include "kernel.h"
#include "syscall.h"

#include <unistd.h>

typedef unsigned long (*sys_call_t)(struct hctx *hctx,
									unsigned long arg1, unsigned long arg2,
									unsigned long arg3, unsigned long arg4,
									void *rest);

#define SC_TRAMPOLINE0(ret, name)                                                                                                                     \
	ret sys_##name(struct hctx *);                                                                                                                    \
	static unsigned long sys_tr_##name(struct hctx *hctx, unsigned long arg1, unsigned long arg2, unsigned long arg3, unsigned long arg4, void *rest) \
	{                                                                                                                                                 \
		return (ret)sys_##name(hctx);                                                                                                                 \
	}
#define SC_TRAMPOLINE1(ret, name, type1, name1)                                                                                                       \
	ret sys_##name(struct hctx *, type1);                                                                                                             \
	static unsigned long sys_tr_##name(struct hctx *hctx, unsigned long arg1, unsigned long arg2, unsigned long arg3, unsigned long arg4, void *rest) \
	{                                                                                                                                                 \
		return (ret)sys_##name(hctx, (type1)arg1);                                                                                                    \
	}
#define SC_TRAMPOLINE2(ret, name, type1, name1, type2, name2)                                                                                         \
	ret sys_##name(struct hctx *, type1, type2);                                                                                                      \
	static unsigned long sys_tr_##name(struct hctx *hctx, unsigned long arg1, unsigned long arg2, unsigned long arg3, unsigned long arg4, void *rest) \
	{                                                                                                                                                 \
		return (ret)sys_##name(hctx, (type1)arg1, (type2)arg2);                                                                                       \
	}
#define SC_TRAMPOLINE3(ret, name, type1, name1, type2, name2, type3, name3)                                                                           \
	ret sys_##name(struct hctx *, type1, type2, type3);                                                                                               \
	static unsigned long sys_tr_##name(struct hctx *hctx, unsigned long arg1, unsigned long arg2, unsigned long arg3, unsigned long arg4, void *rest) \
	{                                                                                                                                                 \
		return (ret)sys_##name(hctx, (type1)arg1, (type2)arg2, (type3)arg3);                                                                          \
	}
#define SC_TRAMPOLINE4(ret, name, type1, name1, type2, name2, type3, name3, type4, name4)                                                             \
	ret sys_##name(struct hctx *, type1, type2, type3, type4);                                                                                        \
	static unsigned long sys_tr_##name(struct hctx *hctx, unsigned long arg1, unsigned long arg2, unsigned long arg3, unsigned long arg4, void *rest) \
	{                                                                                                                                                 \
		return (ret)sys_##name(hctx, (type1)arg1, (type2)arg2, (type3)arg3, (type4)arg4);                                                             \
	}
#define SC_TRAMPOLINE5(ret, name, type1, name1, type2, name2, type3, name3, type4, name4, type5, name5)                                               \
	ret sys_##name(struct hctx *, type1, type2, type3, type4, void *);                                                                                \
	static unsigned long sys_tr_##name(struct hctx *hctx, unsigned long arg1, unsigned long arg2, unsigned long arg3, unsigned long arg4, void *rest) \
	{                                                                                                                                                 \
		return (ret)sys_##name(hctx, (type1)arg1, (type2)arg2, (type3)arg3, (type4)arg4, rest);                                                       \
	}
#define SC_TRAMPOLINE(name, ret, n, ...) \
	SC_TRAMPOLINE##n(ret, name, ##__VA_ARGS__)
SYSCALL_X(SC_TRAMPOLINE)

/*
	SC_TRAMPOLINE(execl, int, 2, const char*, path, char *const *, argv) -->
	SC_TRAMPOLINE2(int, execl, 2, const char*, path, char *const *, argv) -->

	int sys_fork(struct hctx*); \
	static unsigned long sys_tr_fork(struct hctx *hctx, unsigned long arg1, unsigned long arg2, unsigned long arg3, unsigned long arg4, void *rest) { \
		return (int) sys_fork(hctx); \
	}
*/
#undef SC_TRAMPOLINE0
#undef SC_TRAMPOLINE1
#undef SC_TRAMPOLINE2
#undef SC_TRAMPOLINE3
#undef SC_TRAMPOLINE4
#undef SC_TRAMPOLINE5
#undef SC_TRAMPOLINE

#define SC_TABLE_ITEM(name, ...) sys_tr_##name,
static const sys_call_t sys_table[] = {
	SYSCALL_X(SC_TABLE_ITEM)};
#undef SC_TABLE_ITEM

void syscall_bottom(struct hctx *hctx)
{
	int a = hctx->rax;
	hctx->rax = sys_table[a](hctx, hctx->rbx, hctx->rcx, hctx->rdx, hctx->rsi, (void *)hctx->rdi);
	//printf("HCTX->RAX after sys_fork: %d \n", hctx->rax);
}

int sys_print(struct hctx *hctx, char *str, int len)
{
	return write(1, str, len);
}