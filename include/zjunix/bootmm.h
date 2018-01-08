#ifndef BOOTMEM_H
#define BOOTMEM_H

#define PAGE_SHIFT 12 //4k per page
#define PAGE_FREE 0x00
#define PAGE_USED 0xff
#define MAX_INFO 10 // max number of bootmm


enum mm_usage
{
    _MM_KERNEL,  //belongs to kernal, only visited in kernal mode
    _MM_MMMAP,   //used to make bit map
    _MM_VGABUFF, //used for vga buffer, only visited in kernal mode
    _MM_PDTABLE,
    _MM_PTABLE,  //used for paging
    _MM_DYNAMIC, //can be used for dynamic allocating
    _MM_RESERVED,
    _MM_COUNT //number of memory type
};

// record each part of memory and its usage
struct bootmm_info
{
    unsigned int start_pn; //start page number
    unsigned int end_pn;   //end page number
    unsigned int type;     //mm_usage
};

struct bootmm
{
    unsigned int phy_size; // the total physical memory (byte)
    unsigned int max_pn;   // the max page number
    unsigned char *s_map;  // bit map's start addr
    unsigned char *e_map;  //bit map's end addr
    unsigned int last_alloc;
    unsigned int count;                // get number of infos stored in bootmm now
    struct bootmm_info info[MAX_INFO]; // array to record the mm information
};

extern struct bootmm bmm;

//set memory information(start, end, type)
void set_mminfo(struct bootmm_info *info, unsigned int start, unsigned int end, unsigned int type);

//insert in to bmm
unsigned int insert_mminfo(struct bootmm *mm, unsigned int start, unsigned int end, unsigned int type);

//remove from bmm
unsigned int remove_mminfo(struct bootmm *mm, unsigned int index);

// initialize the bootmm
void init_bootmm();

//print bmm's information
void bootmap_info();

// find pages
unsigned char *find_pages(unsigned int count, unsigned int s_pn, unsigned int e_pn, unsigned int align_pn);

// set the bit maps
void set_maps(unsigned int s_pn, unsigned int cnt, unsigned char value);

// alloc pages
unsigned char *bootmm_alloc_pages(unsigned int size, unsigned int type, unsigned int align);

// split segment in bmm
unsigned int split_mminfo(struct bootmm *mm, unsigned int index, unsigned int split_start);

// free pages in bmm
unsigned int bootmm_free_pages(unsigned int start, unsigned int size);

#endif