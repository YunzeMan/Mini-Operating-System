#include "pid.h"

#include <driver/vga.h>
#include <intr.h>
#include <zjunix/syscall.h>
#include <zjunix/utils.h>
#include <zjunix/pc.h>
#include <zjunix/time.h>
#include <zjunix/slab.h>
#include <zjunix/buddy.h>

// The 8-level task queue, higher number stands for higher priority
pidmap_t pid_map;

int last_pid = -1;


/* Initiate pid part 
 * 
 *Function init_pc initiates the pid functions of the OS. It set last allocated pid to -1, free number of pids to MAX
 *and memset the pid page.  
 *Further operations can be added.
 *@No parameters
 *@No return value
 */
void init_pid() 
{
    int i;
    last_pid = -1;
    pid_map.nr_free = PID_MAX_DEFAULT;
    for (i = 0; i < PAGE_SIZE; i++)
        pid_map.page[i] = 0;
}

//这个函数的作用主要是将offset在pidmap变量当中相应的位置为1，也就是申请到一个pid号之后，修改位标志。其中addr是pidmap.page变量的地址。
int test_and_set_bit(int offset, void *addr)
{
    unsigned long mask = 1UL << (offset & (sizeof(unsigned long) * BITS_PER_BYTE - 1));
    unsigned long *p = ((unsigned long*)addr) + (offset >> (sizeof(unsigned long) + 1));
    unsigned long old = *p;
    *p = old | mask;
    return (old & mask) != 0;
}

//这个函数的作用主要是将offset在pidmap变量当中相应的位置为0，也就是释放一个pid号之后，修改位标志。其中addr是pidmap.page变量的地址。
void clear_bit(int offset, void *addr)
{
    unsigned long mask = 1UL << (offset & (sizeof(unsigned long) * BITS_PER_BYTE - 1));//取offset的后31位数据,并左移
    unsigned long *p = ((unsigned long*)addr) + (offset >> (sizeof(unsigned long) + 1));//+优先级高于>>
    unsigned long old = *p;
    *p = old & ~mask;
}

/* Find the next allocatable pid
 * 
 *Function find_next_zero_bit finds the next 0 pid (allocatable pid). 
 *@param addr The address of the page map
 *@param size The size of a page
 *@param offset The finding begin position 
 *@return value is return state
 */
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

// Core function, allocate a valid pid.
int alloc_pidmap()
{
    int pid = last_pid + 1;
    int offset = pid & BITS_PER_PAGE_MASK; // Use & Operation to take call of overflow
    
    if (!pid_map.nr_free)
    {
        return -1;
    }
    offset = find_next_zero_bit(&pid_map.page, BITS_PER_PAGE, offset);
    if (BITS_PER_PAGE != offset && !test_and_set_bit(offset, &pid_map.page))
    {
        --pid_map.nr_free;
        last_pid = offset;
        return offset;
    }
    return -1;
}

// Deallocate a pid
void free_pidmap(int pid)
{
    int offset = pid & BITS_PER_PAGE_MASK;
    pid_map.nr_free++;
    clear_bit(offset, &pid_map.page);
}

void test_pidmap()
{
    int i, pid;
    for (i = 0; i < 10; i++)
    {
        pid = fork();
        if (pid < 0) {
            kernel_printf("  fork failed!\n");
        }
        else if (pid == 0) {
        // In child process
            exit();
        } 
    }

    pid = fork();
    if (pid < 0) {
        kernel_printf("  fork failed!\n");
    }
    else if (pid == 0) {
    // In child process
        kernel_printf("  After %d allocation, call ps:\n", i);
        print_proc();
        exit();
    } 
    exit();
}