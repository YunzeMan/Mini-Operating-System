#include "pid.h"

#include <driver/vga.h>
#include <intr.h>
#include <zjunix/syscall.h>
#include <zjunix/utils.h>

#define PID_MAX 56

// The 8-level task queue, higher number stands for higher priority
pidmap_t pid_map;

int last_pid = -1;


/*offset对应于一个要处理的pid编号。
offset & 31只保留后5位的值，然后1左移这个值的大小位，这样的话，我们就在某一行中找到了具体的对应的那一位了。
offset右移５位，表示将offset的后5位屏蔽掉了。这样的话，我们就可以找到对应的具体的某一行地址了。*/

/*
pid / 8， int a=(pid/8)，将得到的a的值作为pidmap.page的下标，也就是pidmap.page[a]，然后用int b=(pid%8)得到对应位的0—7之间的编号，这样的话，就唯一确定了一个pid编号在pidmap.page中的具体位置。

内核当中是通过移位操作来实现的。我们可以抽象出一张表，这个表有32列，1024行，这个刚好是一个页的大小。这个时候，可以通过pid的后5位值（变化范围在0—31之间）来确定在某一行的具体的列。通过pid的高27位（pid本身是32位的）来表示在具体的某一 行。所以我们可以通过移位操作来实现。
这 里为什么每一行要设置成32列，而不是8列，或者是64列呢？我是这么考虑的，这个算法是在32位机子上运行的，如果是32位的话，刚好是一个 unsigned int所占的空间的大小，这样对它的地址的加一操作会跳跃4个字节，和每行8列进行对比的话，这个操作更能够减少加法操作的次数。


    unsigned long mask = 1UL << (offset & (sizeof(unsigned long) * BITS_PER_BYTE - 1));
    unsigned long *p = ((unsigned long*)addr) + (offset >> (sizeof(unsigned long) + 1));

简化：

    unsigned long mask = 1UL << (offset & 31);
    unsigned long *p = ((unsigned long*)addr) + (offset >> 5);


*/

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

//从offset开始，找下一个是0（也就是可以分配）的pid号。其中addr是pidmap.page变量的地址，size是一个页的大小。
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
    if (pid >= PID_MAX)
		pid = RESERVED_PIDS;
    int offset = pid & BITS_PER_PAGE_MASK;//把offset的最高为变为0，其他的不变
    
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