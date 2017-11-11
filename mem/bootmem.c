//TODO: set include
#include "bootmem.h"

struct bootmm bmm;
char *mem_msg[] = {"Kernel code/data", "Mm Bitmap", "Vga Buffer", "Kernel page directory", "Kernel page table", "Dynamic", "Reserved"};

// set the content of struct bootmm_info
void set_mminfo(struct bootmm_info *info, unsigned int start, unsigned int end, unsigned int type) 
{
    info->start_pfn = start;
    info->end_pfn = end;
    info->type = type;
}

unsigned int insert_mminfo(struct bootmm *mm, unsigned int start, unsigned int end, unsigned int type) 
{    
/*
* return value list:
*		0 -> insert_mminfo failed
*		1 -> insert non-related mm_segment
*		2 -> insert forward-connecting segment
*		4 -> insert next-connecting segment
*		6 -> insert forward-connecting segment to last position
*		7 -> insert bridge-connecting segment(remove_mminfo is called for deleting
*/    
    unsigned int i;
    for (i = 0; i < mm->cnt_infos; i++) {
        if (mm->info[i].type != type)
            continue;  
        if (mm->info[i].end_pfn == start - 1) {
            // new-in mm is connecting to the forwarding one
            if ((i + 1) < mm->cnt_infos) {
                // current info is still not the last segment
                if (mm->info[i + 1].type != type) { 
                // if can not merge to the next seg 
                    mm->info[i].end_pfn = end;
                    return 2;
                } 
                else if (mm->info[i + 1].start_pfn - 1 == end) {
                //if can merge to the next
                    mm->info[i].end_pfn = mm->info[i + 1].end_pfn;
                    if(remove_mminfo(mm, i + 1))
                        //remove the next seg since merge together
                        return 7;
                }    
            } else {  
                // current info is the last segment, there is no next seg
                // extend the last segment to containing the new-in mm
                mm->info[i].end_pfn = end;
                return 6;
            }
        }
        if (mm->info[i].start_pfn - 1 == end) {
            // new-in mm is connecting to the next one
            mm->info[i].start_pfn = start;
            return 4;
        }
    }
    if (mm->cnt_infos >= MAX_INFO)
        return 0;  // cannot insert
    set_mminfo(mm->info + mm->cnt_infos, start, end, type);
    mm->cnt_infos++;
    return 1;  // individual segment(non-connecting to any other)
}

unsigned int remove_mminfo(struct bootmm *mm, unsigned int index) 
{
/*
* return value list:
*		0 -> remove failed
*		1 -> remove succeeded		
*/ 
    unsigned int i;
    if (index >= mm->cnt_infos)
        return 0;
    for (i = (index + 1); i < mm->cnt_infos; i++) {
         mm->info[i - 1] = mm->info[i];
    }    
    mm->cnt_infos--;
    return 1;
}

unsigned char bootmmmap[MACHINE_MMSIZE >> PAGE_SHIFT];

void init_bootmm() 
{
//initialization for bootmem
    
    //TODO: implement memset
    kernel_memset(&bmm, 0, sizeof(bmm));
    
    unsigned int index;
    unsigned char *t_map;
    
    //TODO: implement get_phymm_size()
    bmm.phymm = get_phymm_size();
    bmm.max_pfn = bmm.phymm >> PAGE_SHIFT;
    
    bmm.s_map = bootmmmap;
    bmm.e_map = bootmmmap + sizeof(bootmmmap);

    bmm.cnt_infos = 0;

    //TODO: implement memset
    kernel_memset(bmm.s_map, PAGE_FREE, bmm.e_map - bmm.s_map); //set the bit map to used

    insert_mminfo(&bmm, 0, (unsigned int)(KERNAL_MMSIZE - 1), _MM_KERNEL);
    //insert_mminfo(&bmm, (unsigned int)(bmm.s_map), (unsigned int)(bmm.e_map - 1), _MM_MMMAP);
    
    bmm.last_alloc = (((unsigned int)(16*1024*1024) >> PAGE_SHIFT) - 1);

    for (index = 0; index < (KERNAL_MMSIZE) >> PAGE_SHIFT; index++) {
        bmm.s_map[index] = PAGE_FREE;
    }
}

void bootmap_info() 
{
    unsigned int index;
    //TODO: implememnt printf
    kernel_printf("Mem Map:\n");
    for (index = 0; index < bmm.cnt_infos; index++) {
        kernel_printf("\t%x-%x : %s\n", bmm.info[index].start_pfn, bmm.info[index].end_pfn, mem_msg[bmm.info[index].type]);
    }
}

unsigned char* find_pages(unsigned int count, unsigned int s_pfn, unsigned int e_pfn, unsigned int align_pfn)
{
/*
 * This function is to find sequential page_cnt number of pages to allocate
 * @param count : the number of pages requested
 * @param s_pfn    : the allocating begin page frame node
 * @param e_pfn	   : the allocating end page frame node
 * return value  = 0 :: allocate failed, else return index(page start)
 */
    unsigned int index, tmp;
    unsigned int cnt;

    s_pfn += (align_pfn - 1);  
    s_pfn &= ~(align_pfn - 1);

    for (index = s_pfn; index < e_pfn; ) {
        if (bmm.s_map[index] == PAGE_USED) {
            ++index;
            continue;
        }
        cnt = count;
        tmp = index;
        while (cnt) {
            if (tmp >= e_pfn)
                return 0;
            // reaching end, but allocate request still cannot be satisfied

            if (bmm.s_map[tmp] == PAGE_FREE) {
                tmp++;  // find next possible free page
                cnt--;
            }
            if (bmm.s_map[tmp] == PAGE_USED) {
                break;
            }
        }
        if (cnt == 0) {  // cnt = 0 indicates that the specified page-sequence found
            bmm.last_alloc = tmp - 1;
            set_maps(index, count, PAGE_USED);
            return (unsigned char *)(index << PAGE_SHIFT);
        } else {
            index = tmp + align_pfn;  // there will be no possible memory space
                                      // to be allocated before tmp
        }
    }
    return 0;
}

void set_maps(unsigned int s_pfn, unsigned int cnt, unsigned char value) 
{
/*
 * set value of page-bitmap-indicator
 * @param s_pfn	: page frame start node
 * @param cnt	: the number of pages to be set
 * @param value	: the value to be set
 */
    while (cnt) {
        bmm.s_map[s_pfn] = (unsigned char)value;
        --cnt;
        ++s_pfn;
    }
}

unsigned char *bootmm_alloc_pages(unsigned int size, unsigned int type, unsigned int align) 
{
/*
* provide space 
* @param size : required memory size 
* @param type : type of required memory size
* @param align : slign the page
* return value : 0 :: allocate failed, else return index(page start)
*/
    unsigned int index, tmp;
    unsigned int cnt, t_cnt;
    unsigned char *res;

    size += ((1 << PAGE_SHIFT) - 1);
    size &= (~((1 << PAGE_SHIFT) - 1));
    cnt = size >> PAGE_SHIFT;

    // in normal case, going forward is most likely to find suitable area
    res = find_pages(cnt, bmm.last_alloc + 1, bmm.max_pfn, align >> PAGE_SHIFT);
    if (res) {
        if(insert_mminfo(&bmm, (unsigned int)res, (unsigned int)res + size - 1, type))
            return res;
    }

    // when system request a lot of operations in booting, then some free area
    // will appear in the front part
    res = find_pages(cnt, 0, bmm.last_alloc, align >> PAGE_SHIFT);
    if (res) {
        if(insert_mminfo(&bmm, (unsigned int)res, (unsigned int)res + size - 1, type))
            return res;
    }
    return 0;  // not found or insert failed, return NULL
}



unsigned int split_mminfo(struct bootmm *mm, unsigned int index, unsigned int split_start) 
{
/* get one sequential memory area to be split into two parts
 * (set the former one.end = split_start-1)
 * (set the latter one.start = split_start)
 */
    unsigned int start, end;
    unsigned int tmp;

    start = mm->info[index].start_pfn;
    end = mm->info[index].end_pfn;
    split_start &= (~((1 << PAGE_SHIFT) - 1));

    if ((split_start <= start) || (split_start >= end))
        return 0;  // split_start out of range
    
    if (mm->cnt_infos == MAX_INFO)
        return 0;  // number of segments are reaching max, cannot alloc anymore segments

    // using copy and move, to get a mirror segment of mm->info[index]
    for (tmp = mm->cnt_infos - 1; tmp >= index; --tmp) {
        mm->info[tmp + 1] = mm->info[tmp];
    }
    mm->info[index].end_pfn = split_start - 1;
    mm->info[index + 1].start_pfn = split_start;
    mm->cnt_infos++;
    return 1;
}

void bootmm_free_pages(unsigned int start, unsigned size)
{
    unsigned int index, cnt;

    size &= ~((1 << PAGE_SHIFT) - 1);
    cnt = size >> PAGE_SHIFT
    if(!cnt)
        return;
    
    start &= ~((1 << PAGE_SHIFT) - 1);
    for(index = 0; index < bmm.cnt_infos; index++){
        if(bmm.info[index].end_pfn < start)
            continue;
        if(bmm.info[index].start_pfn > start)
            continue;
        if(start + size - 1 > bmm.info[index].end_pfn)
            continue;
        break;
    }
    if(index == bmm.cnt_infos){
        //TODO: implememnt printf
        kernel_printf("bootmm_free_pages: not alloc space(%x:%x)\n", start, size);
        return;
    }

    set_maps(start >> PAGE_SHIFT, cnt, PAGE_FREE);
    if(bmm.info[index].start_pfn == start){
        if(bmm.info[index].end_pfn == (start + size - 1))
            remove_mminfo(&bmm, index);
        else
            set_mminfo(&(bmm.info[index]), start + size, bmm.info[index].end_pfn, bmm.info[index].type);            
    }
    else{
        if(bmm.info[index].end_pfn == (start + size -1))
            set_mminfo(&(bmm.info[index]), bmm.info[index].start_pfn, start - 1, bmm.info[index].type);
        else{
            split_mminfo(&bmm, index, start);
            split_mminfo(&bmm, index + 1, start + size);
            remove_mminfo(&bmm, index + 1);
        }
    }

}