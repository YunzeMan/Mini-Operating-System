#include "slab.h"

//TODO: THIS SHOULD NOT BE THERE
#define KERNEL_ENTRY 0x80000000

#define KMEM_ADDR(PAGE, BASE) ((((PAGE) - (BASE)) << PAGE_SHIFT) | 0x80000000)

/*
 * one list of PAGE_SHIFT(now it's 12) possbile memory size
 * 96, 192, 8, 16, 32, 64, 128, 256, 512, 1024, (2 undefined)
 * in current stage, set (2 undefined) to be (4, 2048)
 */
struct kmem_cache kmalloc_caches[10];

static unsigned int size_kmem_cache[10] = {96, 192, 8, 16, 32, 64, 128, 256, 512, 1024};

//TODO: should be put in utils.c
unsigned int is_bound(unsigned int val, unsigned int bound) {
    return !(val & (bound - 1));
}

void init_slab()
{
    unsigned int order;
    for (order = 0; order < 10; order++) {
        init_each_slab(&(kmalloc_caches[order]), size_kmem_cache[order]);
    }

    //TODO: print
    kernel_printf("  Setup Slub ok :\n");
    kernel_printf("  \tcurrent slab cache size list:\n\t");
    for (order = 0; order < 10; order++) {
        kernel_printf("  %x ", kmalloc_caches[order].objsize);
    }
    kernel_printf("  \n");
}

void init_each_slab(struct kmem_cache *cache, unsigned int size) 
{
    cache->objsize = size;
    cache->objsize += (SIZE_INT - 1);
    cache->objsize &= ~(SIZE_INT - 1);
    //TODO: orginal here is wrong!
    cache->size = cache->objsize + sizeof(void *);  // add one char as mark(available)
    cache->offset = cache->objsize
    
    init_kmem_cpu(&(cache->cpu));
    init_kmem_node(&(cache->node));
}

void init_kmem_cpu(struct kmem_cache_cpu *kcpu) {
    kcpu->page = 0;
    kcpu->freeobj = 0;
}

void init_kmem_node(struct kmem_cache_node *knode) {
    INIT_LIST_HEAD(&(knode->full));
    INIT_LIST_HEAD(&(knode->partial));
}

void format_slabpage(struct kmem_cache *cache, struct page *page)
 {
    unsigned char *moffset = (unsigned char *)KMEM_ADDR(page, pages);  // physical addr
    struct slab_head *s_head = (struct slab_head *)moffset;
    unsigned int *ptr;
    unsigned int remaining = (1 << PAGE_SHIFT);

    set_flag(page, _PAGE_SLAB);
    do {
        ptr = (unsigned int *)(moffset + cache->offset);
        moffset += cache->size;
        *ptr = (unsigned int)moffset;
        remaining -= cache->size;
    } while (remaining >= cache->size);

    *ptr = (unsigned int)moffset & ~((1 << PAGE_SHIFT) - 1);
    s_head->end_ptr = (void **)ptr;
    s_head->nr_objs = 0;

    //not using the first space (br used to store the slab_head's information)
    cache->cpu.page = page;
    cache->cpu.freeobj = (void **)(*ptr + cache->offset);
    page->virtual = (void *)cache;
    page->slabp = (unsigned int)(*(cache->cpu.freeobj));
}


void *slab_alloc(struct kmem_cache *cache) {
    struct slab_head *s_head;
    void *object = 0;
    struct page *newpage;

    //check whether there is frrobj
    if (cache->cpu.freeobj)
        object = *(cache->cpu.freeobj);

slalloc_check:
    //check if the freeobj is in the boundary situation
    if (is_bound((unsigned int)object, (1 << PAGE_SHIFT))) {
        // if the page is full, move this page to full 
        if (cache->cpu.page) {
            list_add_tail(&(cache->cpu.page->list), &(cache->node.full));
        }
        
        //if there is no partial used page
        if (list_empty(&(cache->node.partial))) {
            // call the buddy system to allocate one more page to be slab-cache
            newpage = __alloc_pages(0);  // get bplevel = 0 page === one page
            if (!newpage) {
                // allocate failed, memory in system is used up
                //TODO: implement print
                kernel_printf("  ERROR: slab request one page in cache failed\n");
                while (1);
            }
            //TODO: implement print
            kernel_printf("  \tnew page, index: %x \n", newpage - pages);

            // using standard format to shape the new-allocated page,
            // set the new page to be cpu.page
            format_slabpage(cache, newpage);
            object = *(cache->cpu.freeobj);
            // as it's newly allocated no check may be need
            goto slalloc_check;
        }

        // get the header of the cpu.page(struct page)
        cache->cpu.page = container_of(cache->node.partial.next, struct page, list);
        list_del(cache->node.partial.next);
        object = (void *)(cache->cpu.page->slabp);
        cache->cpu.freeobj = (void **)((unsigned char *)object + cache->offset);
        goto slalloc_check;
    }

    //update the cache information
    cache->cpu.freeobj = (void **)((unsigned char *)object + cache->offset);
    cache->cpu.page->slabp = (unsigned int)(*(cache->cpu.freeobj));
    
    s_head = (struct slab_head *)KMEM_ADDR(cache->cpu.page, pages); 
    
    ++(s_head->nr_objs);


    // slab may be full after this allocation
    if (is_bound(cache->cpu.page->slabp, 1 << PAGE_SHIFT)) {
        list_add_tail(&(cache->cpu.page->list), &(cache->node.full));
        init_kmem_cpu(&(cache->cpu));
    }
    return object;
}

// find the best-fit slab system for (size)
unsigned int get_slab(unsigned int size) {
    unsigned int i;
    unsigned int bf_index = 9;             
    for (i = 0; i < 10; i++) {
        if ((kmalloc_caches[i].objsize >= size)) {
            bf_index = i;
        }
    }
    return bf_index;
}

void *kmalloc(unsigned int size) {
    struct kmem_cache *cache;
    unsigned int bf_index;

    if (!size)
        return 0;

    if (size > kmalloc_caches[9].objsize) {
        size += (1 << PAGE_SHIFT) - 1;
        size &= ~((1 << PAGE_SHIFT) - 1);
       // return (void *)(KERNEL_ENTRY | (unsigned int)alloc_pages(size >> PAGE_SHIFT));
       unsigned int addr =  (unsigned int)alloc_pages(size >> PAGE_SHIFT);
       if(!addr){
        //TODO: print
        kernel_printf("  ERROR: No available page\n"); 
        while(1);
       }
       return (void *)(KERNEL_ENTRY | addr);
    }

    bf_index = get_slab(size);
    if (bf_index >= 10) {
        //TODO: print
        kernel_printf("  ERROR: No available slab\n");
        while (1);
    }
    return (void *)(KERNEL_ENTRY | (unsigned int)slab_alloc(&(kmalloc_caches[bf_index])));
}

void slab_free(struct kmem_cache *cache, void *object) 
{
/*
* @param : object : the object need to be released 's start address
*/
    struct page *opage = pages + ((unsigned int)object >> PAGE_SHIFT);
    unsigned int *ptr;
    struct slab_head *s_head = (struct slab_head *)KMEM_ADDR(opage, pages);

    if (!(s_head->nr_objs)) {
        kernel_printf("  ERROR : slab_free error!\n");
        while (1);
    }

    ptr = (unsigned int *)((unsigned char *)object + cache->offset);
    *ptr = *((unsigned int *)(s_head->end_ptr));
    *((unsigned int *)(s_head->end_ptr)) = (unsigned int)object;
    --(s_head->nr_objs);

    if (list_empty(&(opage->list)))
        return;

    if (!(s_head->nr_objs)) {
        __free_pages(opage, 0);
        return;
    }

    list_del_init(&(opage->list));
    list_add_tail(&(opage->list), &(cache->node.partial));
}

void kfree(void *obj) {
    struct page *page;

    obj = (void *)((unsigned int)obj & (~KERNEL_ENTRY));
    page = pages + ((unsigned int)obj >> PAGE_SHIFT);
    if (!(page->flag == _PAGE_SLAB))
        return free_pages((void *)((unsigned int)obj & ~((1 << PAGE_SHIFT) - 1)), page->bplevel);

    return slab_free(page->virtual, obj);
}
