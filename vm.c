#define _GNU_SOURCE

#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include "malloc.h"

#include "vm.h"


static int g_memfd = -1;
static unsigned g_memsize;
static off_t offset = 0;
static struct map_range rangepool[16];
static int range_n;
static struct map_range* curr_brked = NULL;

int vmbrk(void *addr) {
	if (MAP_FAILED == mmap(USERSPACE_START,
			addr - USERSPACE_START,
			PROT_READ | PROT_WRITE | PROT_EXEC,
			MAP_FIXED | MAP_SHARED,
			g_memfd, offset)) {
		perror("mmap g_memfd");
		return -1;
	}
	
	int page_cnt = ((addr - USERSPACE_START) % VM_PAGESIZE == 0)? 
								(addr - USERSPACE_START) / VM_PAGESIZE :
								(addr - USERSPACE_START) / VM_PAGESIZE + 1;
	struct map_range *mr = &rangepool[range_n];
	mr->begin = offset;
	mr->end = offset + addr - USERSPACE_START - 1;
	curr_brked = mr;
	range_n++;
	
	offset += (page_cnt * VM_PAGESIZE);
	return 0;
}

int get_fd() {
	return g_memfd;
}

int get_range_nums() {
	return range_n;
}

struct map_range* get_curr_brk() {
	return curr_brked;
}

int vmprotect(void *start, unsigned len, int prot) {
	int osprot = (prot & VM_EXEC  ? PROT_EXEC  : 0) |
		     (prot & VM_WRITE ? PROT_WRITE : 0) |
		     (prot & VM_READ  ? PROT_READ  : 0);
	if (mprotect(start, len, osprot)) {
		perror("mprotect");
		return -1;
	}
	return 0;
}

int vminit(unsigned size) {
	int fd = memfd_create("mem", 0);
	if (fd < 0) {
		perror("memfd_create");
		return -1;
	}

	if (ftruncate(fd, size) < 0) {
		perror("ftrucate");
		return -1;
	}
	g_memfd = fd;
	g_memsize = size;
	return 0;
}

