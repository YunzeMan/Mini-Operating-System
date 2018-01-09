#include "pid.h"

#include <driver/vga.h>
#include <intr.h>
#include <zjunix/syscall.h>
#include <zjunix/utils.h>

// The 8-level task queue, higher number stands for higher priority
pidmap_t pid_map;

int last_pid = -1;

int test_and_set_bit(int offset, void *addr)
{
    unsigned long mask = 1UL << (offset & (sizeof(unsigned long) * BITS_PER_BYTE - 1));
    unsigned long *p = ((unsigned long*)addr) + (offset >> (sizeof(unsigned long) + 1));
    unsigned long old = *p;
    *p = old | mask;
    return (old & mask) != 0;
}
void clear_bit(int offset, void *addr)
{
    unsigned long mask = 1UL << (offset & (sizeof(unsigned long) * BITS_PER_BYTE - 1));//取offset的后31位数据,并左移
    unsigned long *p = ((unsigned long*)addr) + (offset >> (sizeof(unsigned long) + 1));//+优先级高于>>
    unsigned long old = *p;
    *p = old & ~mask;
}
int find_next_zero_bit(void *addr, int size, int offset)
{
    unsigned long *p;
    unsigned long mask;
    while (offset < size)
    {
        p = ((unsigned long*)addr) + (offset >> (sizeof(unsigned long) + 1));
        mask = 1UL << (offset & (sizeof(unsigned long) * BITS_PER_BYTE - 1));
        if ((~(*p) & mask))
        {
            break;
        }
        ++offset;
    }
    return offset;
}

int alloc_pidmap()
{
    int pid = last_pid + 1;
    int offset = pid & BITS_PER_PAGE_MASK;//把offset的最高为变为0，其他的不变
    
    if (!pidmap.nr_free)
    {
        return -1;
    }
    offset = find_next_zero_bit(&pidmap.page, BITS_PER_PAGE, offset);
    if (BITS_PER_PAGE != offset && !test_and_set_bit(offset, &pidmap.page))
    {
        --pidmap.nr_free;
        last_pid = offset;
        return offset;
    }
    return -1;
}

void free_pidmap(int pid)
{
    int offset = pid & BITS_PER_PAGE_MASK;
    pidmap.nr_free++;
    clear_bit(offset, &pidmap.page);
}