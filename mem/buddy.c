#include "list.h"
#include "lock.h"
#include "bootmem.h"
#include "buddy.h"

unsigned int kernel_start_pfn, kernel_end_pfn;
struct page *pages;
struct buddy_sys buddy;

void init_pages(unsigned int start_pfn, unsigned int end_pfn)
{ 
/*
* init pages' information
*/
    unsigned int i;
    for (i = start_pfn; i < end_pfn; i++) {
        clean_flag(pages + i, -1);
        set_flag(pages + i, _PAGE_RESERVED);
        (pages + i)->reference = 1;
        (pages + i)->virtual = (void *)(-1);
        (pages + i)->bplevel = (-1);
        (pages + i)->slabp = 0;  // initially, the free space is the whole page
        INIT_LIST_HEAD(&(pages[i].list));
    }
}

void buddy_info() 
{
/*
* print buddy information
*/
    unsigned int index;
    //TODO: printf function
    kernel_printf("Buddy-system :\n");
    kernel_printf("\tstart page-frame number : %x\n", buddy.buddy_start_pfn);
    kernel_printf("\tend page-frame number : %x\n", buddy.buddy_end_pfn);
    for (index = 0; index <= MAX_BUDDY_ORDER; ++index) {
        kernel_printf("\t(%x)# : %x frees\n", index, buddy.freelist[index].nr_free);
    }
}

void init_budy()
{
    unsigned int bpsize = sizeof(struct page);
    unsigned char *bp_base;
    unsigned int i;

    bp_base = bootmm_alloc_pages(bpsize * bmm.max_pfn, _MM_KERNEL, 1 << PAGE_SHIFT);
    if (!bp_base) {
        //TODO: print function!  
        kernel_printf("\nERROR : bootmm_alloc_pages failed!\nInit buddy system failed!\n");
        while (1);
    }

    //change addr to "virtue" by adding 0x800
    pages = (struct page *)((unsigned int)bp_base | 0x80000000);

    //init pages
    init_pages(0, bmm.max_pfn);


    //set kernal usage memory (0, 16M)
    kernel_start_pfn = 0;
    kernel_end_pfn = 0;
    for (i = 0; i < bmm.cnt_infos; i++) {
        if (bmm.info[i].end_pfn > kernel_end_pfn)
            kernel_end_pfn = bmm.info[i].end_pfn;
    }
    kernel_end_pfn >>= PAGE_SHIFT;

    buddy.buddy_start_pfn = (kernel_end_pfn + (1 << MAX_BUDDY_ORDER) - 1) &
                            ~((1 << MAX_BUDDY_ORDER) - 1);              // the pages that bootmm using cannot be merged into buddy_sys
    buddy.buddy_end_pfn = bmm.max_pfn & ~((1 << MAX_BUDDY_ORDER) - 1);  // remain 2 pages for I/O

    // init freelists of all bplevels
    for (i = 0; i < MAX_BUDDY_ORDER + 1; i++) {
        buddy.freelist[i].nr_free = 0;
        INIT_LIST_HEAD(&(buddy.freelist[i].free_head));
    }
    buddy.start_page = pages + buddy.buddy_start_pfn;
    
    //TODO: implement lock
    init_lock(&(buddy.lock));

    for (i = buddy.buddy_start_pfn; i < buddy.buddy_end_pfn; i++) {
        __free_pages(pages + i, 0);
    }

    buddy_info();
}

void __free_pages(struct page *page, unsigned int bplevel)
{
/* 
* This Fun is to free a block and merge the continuing pages into a larger blocks
* @param: page  : the first page object of blocks need to be free
* @param: bplevel : the level of blocks
*/
    unsigned int page_idx, buddy_idx;
    unsigned int combined_idx;
    struct page *buddy_page;

    clean_flag(page, -1);
    lockup(&buddy.lock);

    page_idx = page - buddy.start_page;
    
    while(bplevel < MAX_BUDDY_ORDER){
        // compute the next buddy block
        buddy_idx = page_idx ^ (1 << bplevel);
        buddy_page = page + (buddy_idx - page_idx);
        // if they are not the same level, just break
        if (!_is_same_bplevel(buddy_page, bplevel)) {
            break;
        }
        list_del_init(&buddy_page->list);
        --buddy.freelist[bplevel].nr_free;
        set_bplevel(buddy_page, -1);
        combined_idx = buddy_idx & page_idx;
        page += (combined_idx - page_idx);
        page_idx = combined_idx;
        ++bplevel;
    }
    set_bplevel(page, bplevel);
    list_add(&(page->list), &(buddy.freelist[bplevel].free_head));
    ++buddy.freelist[bplevel].nr_free;
    unlock(&buddy.lock);
}


struct page *__alloc_pages(unsigned int bplevel) 
{
/*
* This function is to alloc a block of bplevel
* @param: bplevel : The level of blocks users want
*/
    unsigned int current_order, size;
    struct page *page, *buddy_page;
    struct freelist *free;

    lockup(&buddy.lock);

    for (current_order = bplevel; current_order <= MAX_BUDDY_ORDER; current_order++ ) {
        free = buddy.freelist + current_order;
        if (!list_empty(&(free->free_head))){            
            page = container_of(free->free_head.next, struct page, list);
            list_del_init(&(page->list));
            set_bplevel(page, bplevel);
            set_flag(page, _PAGE_ALLOCED);
            --(free->nr_free);       
            size = 1 << current_order;
            while (current_order > bplevel) {
                --free;
                --current_order;
                size >>= 1;
                buddy_page = page + size;
                list_add(&(buddy_page->list), &(free->free_head));
                ++(free->nr_free);
                set_bplevel(buddy_page, current_order);
            }
            unlock(&buddy.lock);
            return page;
        }        
    }
    unlock(&buddy.lock);
    return 0;   
}

void *alloc_pages(unsigned int bplevel) 
{
    struct page *page = __alloc_pages(bplevel);
    if (!page)
        return 0;
    return (void *)((page - pages) << PAGE_SHIFT);
}

void free_pages(void *addr, unsigned int bplevel) 
{
    __free_pages(pages + ((unsigned int)addr >> PAGE_SHIFT), bplevel);
}

