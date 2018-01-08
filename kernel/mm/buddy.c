#include <driver/vga.h>
#include <zjunix/bootmm.h>
#include <zjunix/buddy.h>
#include <zjunix/list.h>
#include <zjunix/lock.h>
#include <zjunix/utils.h>

unsigned int kernel_start_pn, kernel_end_pn;
struct page *pages;
struct buddy_sys buddy;

void init_pages(unsigned int start_pn, unsigned int end_pn)
{
    // init the pages
    unsigned int i;
    for (i = start_pn; i < end_pn; i++)
    {
        (pages + i)->flag = 0;
        //set_flag(pages + i,);
        //(pages + i)->reference = 1;
        (pages + i)->reference = 0;
        (pages + i)
            ->virtual = (void *)(-1);
        (pages + i)->bplevel = (-1);
        (pages + i)->slabp = 0; // initially, the free space is the whole page
        (pages + i)->nr_objs = 0;
        (pages + i)->end_ptr = 0;
        (pages + i)->freeobj = 0;
        (pages + i)->max_object = 0;
        INIT_LIST_HEAD(&(pages[i].list));
    }
}

void init_buddy()
{
    // init the buddy system
    unsigned int bpsize = sizeof(struct page); // the size of page struct
    unsigned char *bp_base;
    unsigned int i;

    // get a space to record pages' information in the kernel
    bp_base = bootmm_alloc_pages(bpsize * bmm.max_pn, _MM_KERNEL, 1 << PAGE_SHIFT);
    if (!bp_base)
    {
        kernel_printf("\nERROR : bootmm_alloc_pages failed!\nInit buddy system failed!\n");
        while (1)
            ;
    }

    pages = (struct page *)((unsigned int)bp_base | 0x80000000);

    //init pages
    init_pages(0, bmm.max_pn);

    //set kernal usage memory (0, 16M)
    kernel_start_pn = 0;
    kernel_end_pn = 0;
    // find the kernel end page address
    for (i = 0; i < bmm.count; i++)
    {
        if (bmm.info[i].end_pn > kernel_end_pn)
            kernel_end_pn = bmm.info[i].end_pn;
    }
    //transfers to page number
    kernel_end_pn >>= PAGE_SHIFT;

    // set the buddy start page number and end page number
    buddy.buddy_start_pn = (kernel_end_pn + (1 << MAX_BUDDY_ORDER) - 1) &
                           ~((1 << MAX_BUDDY_ORDER) - 1);            // the pages that bootmm using cannot be merged into buddy_sys
    buddy.buddy_end_pn = bmm.max_pn & ~((1 << MAX_BUDDY_ORDER) - 1); // remain 2 pages for I/O

    // init freelists of all bplevels
    for (i = 0; i < MAX_BUDDY_ORDER + 1; i++)
    {
        buddy.freelist[i].count = 0;
        INIT_LIST_HEAD(&(buddy.freelist[i].free_head));
    }

    buddy.start_page = pages + buddy.buddy_start_pn;

    init_lock(&(buddy.lock));

    for (i = buddy.buddy_start_pn; i < buddy.buddy_end_pn; i++)
    {
        __free_pages(pages + i, 0);
    }

    // print the buddy information after init
    buddy_info();
}

void buddy_info()
{
    // print the buddy information
    unsigned int index;
    kernel_printf("Buddy-system :\n");
    kernel_printf("\tstart page-frame number : %x\n", buddy.buddy_start_pn);
    kernel_printf("\tend page-frame number : %x\n", buddy.buddy_end_pn);
    for (index = 0; index <= MAX_BUDDY_ORDER; index++)
    {
        kernel_printf("\t(%x)# : %x frees\n", index, buddy.freelist[index].count);
    }
}

void __free_pages(struct page *page, unsigned int bplevel)
{
    /* 
* This function is to free a block and merge the continuing pages into a larger blocks
* page  : the first page object of blocks need to be free
* bplevel : the level of blocks
*/
    unsigned int page_id, buddy_id;
    unsigned int combined_id;
    struct page *buddy_page;

    page->flag = 0;
    page->reference = 0;
    lockup(&buddy.lock);

    // get the page id in buddy system
    page_id = page - buddy.start_page;

    while (bplevel < MAX_BUDDY_ORDER)
    {
        // compute the next buddy block
        buddy_id = page_id ^ (1 << bplevel);
        buddy_page = page + (buddy_id - page_id);
        // if they are not the same level, just break
        if (!_is_same_bplevel(buddy_page, bplevel))
        {
            break;
        }
        // is find the buddy page, move them from init level, and merge to a higher level at last
        list_del_init(&buddy_page->list);
        // since move out, then count --
        buddy.freelist[bplevel].count--;
        set_bplevel(buddy_page, -1);
        combined_id = buddy_id & page_id;
        page += (combined_id - page_id);
        page_id = combined_id;
        bplevel++;
    }
    // merge to a higher level
    set_bplevel(page, bplevel);
    list_add(&(page->list), &(buddy.freelist[bplevel].free_head));
    buddy.freelist[bplevel].count++;
    unlock(&buddy.lock);
}

struct page *__alloc_pages(unsigned int bplevel)
{
    /*
* This function is to alloc a block of bplevel
* bplevel : The level of blocks users want
* return 0 if there is no pages to alloc
*   else return the page address in kernel controller
*/
    kernel_printf(".....not here.....\n");

    unsigned int current_order, size;
    struct page *page, *buddy_page;
    struct freelist *free;

    lockup(&buddy.lock);

    for (current_order = bplevel; current_order <= MAX_BUDDY_ORDER; current_order++)
    {
        // check freelist of given level to see if there are free block
        free = buddy.freelist + current_order;
        if (!list_empty(&(free->free_head)))
        {
            // if there exist such free pages, remove from the free list
            page = container_of(free->free_head.next, struct page, list);
            list_del_init(&(page->list));
            set_bplevel(page, bplevel);
            // set the flag to declare that the page is in use
            page->flag = 1;
            free->count--;
            // calculate the current pages number per block
            size = 1 << current_order;
            while (current_order > bplevel)
            {
                free--;
                current_order--;
                size >>= 1;
                buddy_page = page + size;
                list_add(&(buddy_page->list), &(free->free_head));
                free->count++;
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
    /*
* function to alloc a page in specific level
*/
    struct page *page = __alloc_pages(bplevel);
    if (!page)
        return 0;
    // use the offset to map to the real address
    return (void *)((page - pages) << PAGE_SHIFT);
}

void free_pages(void *addr, unsigned int bplevel)
{
    /*
* function to free a page from addr
*/
    // if the input address matches the bplevel then free the page
    kernel_printf(".....not here\n");
    if ((pages + ((unsigned int)addr >> PAGE_SHIFT))->flag == 1)
    {
        if ((pages + ((unsigned int)addr >> PAGE_SHIFT))->bplevel == bplevel)
        {
            __free_pages(pages + ((unsigned int)addr >> PAGE_SHIFT), bplevel);
        }
    }
}
