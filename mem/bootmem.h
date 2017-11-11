#ifndef BOOTMEM_H
#define BOOTMEM_H

//TODO: __end is the tail of code/data segment
extern unsigned char __end[];
//TODO: should be put in driver
#define MACHINE_MMSIZE 512*1024*1024
#define KERNAL_MMSIZE 16 * 1024 *1024

enum mm_usage { 
    _MM_KERNEL, //belongs to kernal, only visited in kernal mode
    _MM_MMMAP,  //used to make bit map
    _MM_VGABUFF, //used for vga buffer, only visited in kernal mode
    _MM_PDTABLE, 
    _MM_PTABLE,  //used ofr paging
    _MM_DYNAMIC,  //can be used for dynamic allocating
    _MM_RESERVED, 
    _MM_COUNT   //number of memory type
};

struct bootmm_info {
    unsigned int start_pfn;  //start page number
    unsigned int end_pfn;  //end page number
    unsigned int type;  //mm_usage
};

#define PAGE_SHIFT 12  //4k per page
#define PAGE_FREE 0x00   
#define PAGE_USED 0xff


#define MAX_INFO 10

struct bootmm {
    unsigned int phymm;    // the total physical memory (byte)
    unsigned int max_pfn;  // the max page number
    unsigned char* s_map;  // bit map's begin addr
    unsigned char* e_map;   //bit map's end addr
    unsigned int last_alloc; 
    unsigned int cnt_infos;  // get number of infos stored in bootmm now
    struct bootmm_info info[MAX_INFO];
};

extern struct bootmm bmm;

//set memory information(start, end, type)
void set_mminfo(struct bootmm_info *info, unsigned int start, unsigned int end, unsigned int type);

//insert in to bmm
unsigned int insert_mminfo(struct bootmm* mm, unsigned int start, unsigned int end, unsigned int type);

//remove from bmm
unsigned int remove_mminfo(struct bootmm* mm, unsigned int index);


//TODO: implement get_phymm_size()
void init_bootmm();

//print bmm's information
void bootmap_info();

unsigned char* find_pages(unsigned int count, unsigned int s_pfn, unsigned int e_pfn, unsigned int align_pfn);

void set_maps(unsigned int s_pfn, unsigned int cnt, unsigned char value);

unsigned char *bootmm_alloc_pages(unsigned int size, unsigned int type, unsigned int align) ;

unsigned int split_mminfo(struct bootmm *mm, unsigned int index, unsigned int split_start); 

void bootmm_free_pages(unsigned int start, unsigned int size);
#endif