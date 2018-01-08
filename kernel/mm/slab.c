#include <arch.h>
#include <driver/vga.h>
#include <zjunix/slab.h>
#include <zjunix/utils.h>

#define KMEM_ADDR(PAGE, BASE) ((((PAGE) - (BASE)) << PAGE_SHIFT) | 0x80000000)

// one list of PAGE_SHIFT(now it's 12) possbile memory size 96, 192, 8, 16, 32, 64, 128, 256, 512, 1024
struct kmem_cache kmalloc_caches[10];

// array of each kind of slab page size
static unsigned int size_kmem_cache[10] = {96, 192, 8, 16, 32, 64, 128, 256, 512, 1024};

void init_each_slab(struct kmem_cache *cache, unsigned int size)
{
/*
* function to init each slab
*/
    cache->objsize = size;
    cache->objsize += (SIZE_INT - 1);
    cache->objsize &= ~(SIZE_INT - 1);
    cache->size = cache->objsize + sizeof(void *); // add one char as mark(available)
    // the offset is the object size instead of size
    cache->offset = cache->objsize;

    init_kmem_cpu(&(cache->cpu));
    init_kmem_node(&(cache->node));
}

void init_kmem_cpu(struct kmem_cache_cpu *kcpu)
{
    kcpu->page = 0;
    kcpu->freeobj = 0;
}

void init_kmem_node(struct kmem_cache_node *knode)
{
    INIT_LIST_HEAD(&(knode->full));
    INIT_LIST_HEAD(&(knode->partial));
}

void format_slabpage(struct kmem_cache *cache, struct page *page)
{
    /*
* functions to format each page for slab usage
*/
    unsigned char *kernel_offset = (unsigned char *)KMEM_ADDR(page, pages);
    //struct slab_head *s_head = (struct slab_head *)kernel_offset;
    unsigned int *ptr;
    unsigned int remaining = (1 << PAGE_SHIFT);
    unsigned char *temp = kernel_offset;
    unsigned int max_object = 0;
    // set the page usage for slab
  

    //set_flag(page, _PAGE_SLAB);
   // kernel_printf("reamining: %x\n", remaining);
    do
    {
        ptr = (unsigned int *)(kernel_offset + cache->offset);
        kernel_offset += cache->size;
        *ptr = (unsigned int)kernel_offset;
        remaining -= cache->size;
        //kernel_printf("offset: %x  %x\n", cache->offset, cache->size);
        //kernel_printf("*ptr: %x\n", *ptr);

        //kernel_printf("kernel_offset: %x\n", kernel_offset);
        max_object ++;
       // kernel_printf("reamining: %x\n", remaining);

    } while (remaining >= cache->size);

    *ptr = (unsigned int)kernel_offset & ~((1 << PAGE_SHIFT) - 1);

    // undate the page information
    page->reference = 1;
    //kernel_printf("     %x   %x\n", page->flag, _PAGE_SLAB);
    page->nr_objs = 0;
    page->end_ptr = (void *)(ptr - cache->offset);
    
    cache->cpu.page = page;
    cache->cpu.freeobj = (void *)(temp);
    page->virtual = (void *)cache;
    //page->slabp = (unsigned int)((cache->cpu.freeobj));
    page->max_object = max_object;

    kernel_printf("max object num: %x\n", page->max_object);

    page->freeobj = cache->cpu.freeobj;
}

void *slab_alloc(struct kmem_cache *cache)
{
    //struct slab_head *s_head;
    void *object = 0;
    struct page *newpage;

    if(cache->cpu.page){
        cache->cpu.freeobj = cache->cpu.page->freeobj;
    }
    //check whether there is freeobj
    kernel_printf("cpu.freeobj: %x\n", cache->cpu.freeobj);
    if (cache->cpu.freeobj)
        object = (cache->cpu.freeobj);
    
    
slalloc_check:
    //check if the freeobj is in the boundary situation
    if ((object == 0)||(cache->cpu.page->nr_objs >= cache->cpu.page->max_object))
    {
        // if the page is full, move this page to full
        if (cache->cpu.page)
        {
            list_add_tail(&(cache->cpu.page->list), &(cache->node.full));
        }

        //if there is no partial used page
        if (list_empty(&(cache->node.partial)))
        {
            // call the buddy system to allocate one more page to be slab-cache
            newpage = __alloc_pages(0); // get bplevel = 0 page === one page
            if (!newpage)
            {
                // allocate failed, memory in system is used up
                kernel_printf("ERROR: slab request one page in cache failed\n");
                while (1)
                    ;
            }
            
            //kernel_printf("\tnew page %x , index: %x \n", newpage, newpage - pages);

            // using standard format to shape the new-allocated page,
            // set the new page to be cpu.page
            format_slabpage(cache, newpage);

            //kernel_printf("  %x  %x\n", cache->cpu.page, cache->cpu.page->reference);
           
            object = (cache->cpu.page->freeobj);
            //kernel_printf("object: %x\n", object);
            goto normal;
            // as it's newly allocated no check may be need
        }

        // get the header of the cpu.page(struct page)
        cache->cpu.page = container_of(cache->node.partial.next, struct page, list);

        list_del(cache->node.partial.next);
        
        cache->cpu.freeobj = (void *)(cache->cpu.page->freeobj);
        object = (void *)(cache->cpu.page->freeobj);
        //kernel_printf("object: %x\n", object);
        cache->cpu.page->freeobj = (void *)(*((unsigned int *)(object + cache->offset)));
        cache->cpu.freeobj = cache->cpu.page->freeobj;
        goto slalloc_check;
    }
normal:
    //update the cache information
    //kernel_printf("obj: %x  offset: %x\n",object, cache->offset);
    cache->cpu.page->freeobj = (void *)(*((unsigned int *)(object + cache->offset)));
    cache->cpu.freeobj = cache->cpu.page->freeobj;
    //page->freeobj = cache->cpu.freeobj;
    kernel_printf("next free obj: %x\n", cache->cpu.page->freeobj);
    //cache->cpu.page->freeobj = (void *)((cache->cpu.freeobj));

    //s_head = (struct slab_head *)KMEM_ADDR(cache->cpu.page, pages);
    (cache->cpu.page->nr_objs)++;
    kernel_printf("alloc obj num: %x\n",cache->cpu.page->nr_objs );
    //++(s_head->nr_objs);

    // slab may be full after this allocation
    if (cache->cpu.page->nr_objs >= cache->cpu.page->max_object)
    {
        list_add_tail(&(cache->cpu.page->list), &(cache->node.full));
        init_kmem_cpu(&(cache->cpu));
    }
    return object;
}

// find the best-fit slab system for (size)
unsigned int get_slab(unsigned int size)
{
    unsigned int i;
    unsigned int bf_index = 10;
    for (i = 0; i < 10; i++)
    {
        if ((kmalloc_caches[i].objsize >= size))
        {
            bf_index = i;
            return bf_index;
        }
    }
    return bf_index;
}

void *kmalloc(unsigned int size)
{
    struct kmem_cache *cache;
    unsigned int bf_index;

    if (!size)
        return 0;

    if (size > kmalloc_caches[9].objsize)
    {
        size += (1 << PAGE_SHIFT) - 1;
        size &= ~((1 << PAGE_SHIFT) - 1);
        // return (void *)(KERNEL_ENTRY | (unsigned int)alloc_pages(size >> PAGE_SHIFT));
        unsigned int addr = (unsigned int)alloc_pages(size >> PAGE_SHIFT);
        kernel_printf("test %x\n", addr);
        if (!addr)
        {
            kernel_printf("ERROR: No available page\n");
            while (1) ;
        }
        return (void *)(KERNEL_ENTRY | addr);
    }

    bf_index = get_slab(size);
    kernel_printf("get slab %x\n", bf_index);
    if (bf_index >= 10)
    {
        kernel_printf("ERROR: No available slab\n");
        while (1);
    }
    return (void *)(KERNEL_ENTRY | (unsigned int)slab_alloc(&(kmalloc_caches[bf_index])));
}

void slab_free(struct kmem_cache *cache, void *object, void *temp)
{
/*
* object : the object need to be released 's start address
*/
    kernel_printf("object to free: %x\n", object);
    struct page *temp_page = pages + ((unsigned int)temp >> PAGE_SHIFT);
    kernel_printf("page to free: %x\n", temp_page);
    unsigned int *ptr;
    //struct slab_head *s_head = (struct slab_head *)KMEM_ADDR(opage, pages);

    //kernel_printf("page obj number: %x\n", temp_page->nr_objs);
    if (!(temp_page->nr_objs))
    {
        kernel_printf("ERROR : slab_free error!\n");
        while (1) ;
    }

    ptr = (unsigned int *)((unsigned char *)object + cache->offset);
    

    *ptr = (unsigned int)temp_page->freeobj;
    temp_page->freeobj = object;
    
    
    (temp_page->nr_objs)--;

    if (list_empty(&(temp_page->list)))
        return;

    if (!(temp_page->nr_objs))
    {
        __free_pages(temp_page, 0);
        return;
    }

    list_del_init(&(temp_page->list));
    list_add_tail(&(temp_page->list), &(cache->node.partial));
}

void kfree(void *obj)
{
    struct page *page;
    void * temp = (void *)((unsigned int)obj & (~KERNEL_ENTRY));
    obj = (void *)((unsigned int)obj | (KERNEL_ENTRY));
    kernel_printf("kfree obj: %x\n", obj);
    page = pages + ((unsigned int)temp >> PAGE_SHIFT);
    kernel_printf("kfree page: %x\n", page);
    kernel_printf(" %x\n", page->reference);
    if (!(page->reference == 1))
    {
        return free_pages((void *)((unsigned int)obj & ~((1 << PAGE_SHIFT) - 1)), page->bplevel);
    }

    return slab_free(page->virtual, obj, temp);
}


void init_slab()
{
    unsigned int order;
    for (order = 0; order < 10; order++)
    {
        init_each_slab(&(kmalloc_caches[order]), size_kmem_cache[order]);
    }

    kernel_printf("Setup Slub ok :\n");
    kernel_printf("\tcurrent slab cache size list:\n\t");
    for (order = 0; order < 10; order++)
    {
        kernel_printf("%x ", kmalloc_caches[order].objsize);
    }
    kernel_printf("\n");
}

