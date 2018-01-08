#ifndef SLAB_H
#define SLAB_H

#include <zjunix/list.h>
#include <zjunix/buddy.h>

#define SIZE_INT 4
#define SLAB_AVAILABLE 0x0
#define SLAB_USED 0xff

/* move them to the kernel page struct
struct slab_head
{
    //TODO: original is wrong
    void **end_ptr;  
    unsigned int nr_objs;  //numbers of memory objects that has been allocated
};
*/


// document slab pages
struct kmem_cache_node 
{
    struct list_head partial;  //partial alloc slab pages
    struct list_head full;   //full used slab pages
};

struct kmem_cache_cpu 
{
    //slab page are being allocated
    void *freeobj;  
    struct page *page;  //the page related to slab
};

// slab system
struct kmem_cache {
    unsigned int size;  //every objects actual allocated size
    unsigned int objsize;  //the space the object required
    unsigned int offset;   //the offset from start of th page
    struct kmem_cache_node node;  
    struct kmem_cache_cpu cpu;
    unsigned char name[16];  
};

void init_slab();

void init_each_slab(struct kmem_cache *cache, unsigned int size);

void init_kmem_node(struct kmem_cache_node *knode);

void init_kmem_cpu(struct kmem_cache_cpu *kcpu);

void format_slabpage(struct kmem_cache *cache, struct page *page);

void slab_free(struct kmem_cache *cache, void *object, void *temp);

void *slab_alloc(struct kmem_cache *cache);

void *kmalloc(unsigned int size);

void kfree(void *obj);

#endif