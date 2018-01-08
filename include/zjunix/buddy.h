#ifndef BUDDY_H
#define BUDDY_H

#include <zjunix/list.h>
#include <zjunix/lock.h>

#define _PAGE_RESERVED (1 << 31)
#define _PAGE_ALLOCED (1 << 30)
#define _PAGE_SLAB (1 << 29)



/*
 * struct buddy page is one info-set for the buddy group of pages
 */
struct page 
{
    unsigned int flag;       // the declaration of the usage of this page
    //unsigned int reference;  
    struct list_head list;   // double-way list
    void *virtual;           // default 0x(-1)
    unsigned int bplevel;    // the level of the page
    unsigned int slabp;      //if the page is used by slab system, then slabp represents the base-addr of free space

//TODO : move the slab head here
    void *end_ptr;  
    unsigned int nr_objs;  //numbers of memory objects that has been allocated
};


#define PAGE_SHIFT 12
// 2k per page
#define MAX_BUDDY_ORDER 4
// max 2^4 continuous pages

// the list of unused pages
struct freelist
{
    unsigned int count; // the number of free pages
    struct list_head free_head;
};

// the buddy system
struct buddy_sys
{
    unsigned int buddy_start_pn;  // buddy system start page number
    unsigned int buddy_end_pn;  // buddy system end page number
    struct page *start_page;  // the start page address of buddy usage in control segment
    struct lock_t lock;
    struct freelist freelist[MAX_BUDDY_ORDER + 1];
};

extern struct page *pages;
extern struct buddy_sys buddy;

#define set_flag(page, val) ((*(page)).flag |= (val))
#define clean_flag(page, val) ((*(page)).flag &= ~(val))
#define _is_same_bpgroup(page, bage) (((*(page)).bplevel == (*(bage)).bplevel))
#define _is_same_bplevel(page, lval) ((*(page)).bplevel == (lval))
#define set_bplevel(page, lval) ((*(page)).bplevel = (lval))
#define has_flag(page, val) ((*(page)).flag & val)
#define set_ref(page, val) ((*(page)).reference = (val))
#define inc_ref(page, val) ((*(page)).reference += (val))
#define dec_ref(page, val) ((*(page)).reference -= (val))

// initialize each pages
void init_pages(unsigned int start_pn, unsigned int end_pn);

// initialize the buddy system
void init_buddy();

// print the buddy information
void buddy_info();

//free a block and merge the continuing pages into a larger blocks
void __free_pages(struct page *page, unsigned int bplevel);

struct page* __alloc_pages(unsigned int bplevel);

void free_pages(void *addr, unsigned int bplevel);

//unsigned int get_slab(unsigned int size);

// alloc a page of specific level
void *alloc_pages(unsigned int bplevel);


//TODO: should be in utils
#define container_of(ptr, type, member) ((type*)((char*)ptr - (char*)&(((type*)0)->member)))
#endif