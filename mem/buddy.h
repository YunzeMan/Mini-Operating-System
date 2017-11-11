#ifndef BUDDY_H
#define BUDDY_H

#include "list.h"
#include "lock.h"

#define _PAGE_RESERVED (1 << 31)
#define _PAGE_ALLOCED (1 << 30)
#define _PAGE_SLAB (1 << 29)



/*
 * struct buddy page is one info-set for the buddy group of pages
 */
struct page 
{
    unsigned int flag;       // the declaration of the usage of this page
    unsigned int reference;  //
    struct list_head list;   // double-way list
    void *virtual;           // default 0x(-1)
    
    unsigned int bplevel;    /* the level of the page
                              *
                              * unsigned int sl_objs;
                              * 		represents the number of objects in current
                              * if the page is of _PAGE_SLAB, then bplevel is the sl_objs
                              */
    unsigned int slabp;      /* if the page is used by slab system,
                              * then slabp represents the base-addr of free space
                              */
};


#define PAGE_SHIFT 12
#define MAX_BUDDY_ORDER 4

struct freelist
{
    unsigned int nr_free;
    struct list_head free_head;
};

struct buddy_sys
{
    unsigned int buddy_start_pfn;
    unsigned int buddy_end_pfn;
    struct page *start_page;
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

void init_pages(unsigned int start_pfn, unsigned int end_pfn);

void init_buddy();

void buddy_info();

void __free_pages(struct page *page, unsigned int bplevel);

struct page* __alloc_pages(unsigned int bplevel);

void free_pages(void *addr, unsigned int order);

unsigned int get_slab(unsigned int size);

void *alloc_pages(unsigned int order);


//TODO: should be in utils
#define container_of(ptr, type, member) ((type*)((char*)ptr - (char*)&(((type*)0)->member)))
#endif