#ifndef _ZJUNIX_PID_H
#define _ZJUNIX_PID_H

/* Max number of pid, equal to 0x8000*/
#define PID_MAX_DEFAULT 0x8000
/* Page size is 4K bytes */
#define PAGE_SHIFT 12
/* Shift left to get page_size */
#define PAGE_SIZE (1UL << PAGE_SHIFT)
#define BITS_PER_BYTE 8
#define BITS_PER_PAGE (PAGE_SIZE * BITS_PER_BYTE)
#define BITS_PER_PAGE_MASK (BITS_PER_PAGE - 1)

/* Essential Struct pidmap
 * 
 * The struct pidmap save the state of the pid allocation and the current free pids.
 *
 * With this struct we are able to control pid control. When the number of processes gets higher,
 * this way of controling the pid becomes economical.
 */
typedef struct pidmap{
    unsigned int nr_free;
    char page[PAGE_SIZE];
} pidmap_t;

#define RESERVED_PIDS 10

void init_pid();
int alloc_pidmap();
int test_and_set_bit(int offset, void *addr);
void clear_bit(int offset, void *addr);
int find_next_zero_bit(void *addr, int size, int offset);
void free_pidmap(int pid);
void test_pidmap();

#endif  // !_ZJUNIX_PID_H