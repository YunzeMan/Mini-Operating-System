#include <arch.h>
#include <driver/vga.h>
#include <zjunix/bootmm.h>
#include <zjunix/utils.h>

#define KERNAL_MMSIZE 16 * 1024 *1024

// core part to record the information of memory
struct bootmm bmm;

// the message we can use for printing
char *mem_msg[] = {"Kernel code/data", "Mm Bitmap", "Vga Buffer", "Kernel page directory", "Kernel page table", "Dynamic", "Reserved"};

// bootmap for bootmm
unsigned char bootmap[MACHINE_MMSIZE >> PAGE_SHIFT];

// set the information of one used memory block
void set_mminfo(struct bootmm_info *info, unsigned int start, unsigned int end, unsigned int type)
{
    info->start_pn = start;
    info->end_pn = end;
    info->type = type;
}

unsigned int insert_mminfo(struct bootmm *mm, unsigned int start, unsigned int end, unsigned int type)
{
    /*
* This function insert a memory segment
* return value list:
*		0 -> insert failed
*		1 -> insert non-related segment
*		2 -> insert forward-connecting segment
*		3 -> insert next-connecting segment
*		4 -> insert forward-connecting segment to last position
*		5 -> insert bridge-connecting segment
*/
    unsigned int i;
    for (i = 0; i < mm->count; i++)
    {
        // if the type does not match, just continue
        if (mm->info[i].type != type)
            continue;
        // if the new block is just after the existing block
        if (mm->info[i].end_pn == start - 1)
        {
            // current block is not the last segment
            if ((i + 1) < mm->count)
            {
                // the next existing segment is of different type
                if (mm->info[i + 1].type != type)
                {
                    // merge the new block with the forwarding segment
                    mm->info[i].end_pn = end;
                    return 2;
                }
                else if (mm->info[i + 1].start_pn - 1 == end)
                {
                    //if can merge to the next segment
                    mm->info[i].end_pn = mm->info[i + 1].end_pn;
                    if (remove_mminfo(mm, i + 1))
                        //remove the next segment after merge together
                        return 5;
                }
            }
            else
            {
                // current info is the last segment, there is no next segment
                // extend the last segment to containing the new-in mm
                mm->info[i].end_pn = end;
                return 4;
            }
        }
        // if the new block is just before the existing block
        if (mm->info[i].start_pn - 1 == end)
        {
            mm->info[i].start_pn = start;
            return 3;
        }
    }
    // there does not exist any block to merge
    if (mm->count >= MAX_INFO)
        return 0; // cannot insert since beyond the limit
    // set information of inserting segment
    set_mminfo(mm->info + mm->count, start, end, type);
    mm->count++;
    return 1;
}

unsigned int remove_mminfo(struct bootmm *mm, unsigned int index)
{
    /*
* This function remove a memory segment
* return value list:
*		0 -> remove failed
*		1 -> remove succeeded		
*/
    unsigned int i;
    // if the input index exceed the limit, remove failed
    if (index >= mm->count)
        return 0;
    // remove this segment, and move the array ahead
    for (i = (index + 1); i < mm->count; i++)
    {
        mm->info[i - 1] = mm->info[i];
    }
    mm->count--;
    return 1;
}

//initialization for bootmem
void init_bootmm()
{
    // clear the bmm
    kernel_memset(&bmm, 0, sizeof(bmm));

    unsigned int index;

    // get the physical memory size
    bmm.phy_size = MACHINE_MMSIZE;
    // calculate the max page number
    bmm.max_pn = bmm.phy_size >> PAGE_SHIFT;

    // record the boot map's address
    bmm.s_map = bootmap;
    bmm.e_map = bootmap + sizeof(bootmap);

    // set the bit map free
    kernel_memset(bmm.s_map, PAGE_USED, bmm.e_map - bmm.s_map);

    // the segment count is set to 0
    bmm.count = 0;

    // insert the segment for kernel usage
    insert_mminfo(&bmm, 0, (unsigned int)(KERNAL_MMSIZE - 1), _MM_KERNEL);

    // record the last allocated page number
    bmm.last_alloc = (((unsigned int)(KERNAL_MMSIZE) >> PAGE_SHIFT) - 1);

    for (index = (KERNAL_MMSIZE) >> PAGE_SHIFT; index < bmm.max_pn; index++)
    {
        bmm.s_map[index] = PAGE_FREE;
    }

    bootmap_info();
}

// print the boot map information
void bootmap_info()
{
    unsigned int index;
    kernel_printf("  Mem Map:\n");
    for (index = 0; index < bmm.count; index++)
    {
        kernel_printf("  \t%x-%x : %s\n", bmm.info[index].start_pn, bmm.info[index].end_pn, mem_msg[bmm.info[index].type]);
    }
}

unsigned char *find_pages(unsigned int count, unsigned int s_pn, unsigned int e_pn, unsigned int align_pn)
{
    /*
 * This function is to find continuous number of pages to allocate
 * count : the number of pages requested
 * s_pn : the start page number bound 
 * e_pn : the end page number bound
 * return value  = 0 : allocate failed, 
 *        else return index(page start)
 */
    unsigned int index, tmp;
    unsigned int cnt;

    // align the input page number
    s_pn += (align_pn - 1);
    s_pn &= ~(align_pn - 1);

    for (index = s_pn; index < e_pn;)
    {
        // if current page is used, index ++
        if (bmm.s_map[index] == PAGE_USED)
        {
            ++index;
            continue;
        }
        cnt = count;
        tmp = index;
        while (cnt)
        {
            if (tmp >= e_pn)
                return 0;
            // cannot find memory segment satisfies the requirements

            // check if there exist continuous pages to allocate
            if (bmm.s_map[tmp] == PAGE_FREE)
            {
                tmp++;
                cnt--;
            }
            if (bmm.s_map[tmp] == PAGE_USED)
            {
                break;
            }
        }
        if (cnt == 0)
        { // cnt = 0 indicates that the specified page-sequence found
            //record the last alloc page number
            bmm.last_alloc = tmp - 1;
            // set the bootmm maps
            set_maps(index, count, PAGE_USED);
            return (unsigned char *)(index << PAGE_SHIFT);
        }
        else
        {
            index = tmp + align_pn;
        }
    }
    return 0;
}

void set_maps(unsigned int s_pn, unsigned int cnt, unsigned char value)
{
    /*
 * set value of bitmap
 *  s_pn : start page number
 *  cnt	: the number of pages to be set
 *  value : the value to be set
 */
    while (cnt)
    {
        bmm.s_map[s_pn] = (unsigned char)value;
        --cnt;
        ++s_pn;
    }
}

unsigned char *bootmm_alloc_pages(unsigned int size, unsigned int type, unsigned int align)
{
    /*
* allocate page for bootmm usage
* size : required memory size 
* type : type of required memory size 
* return value = 0 : allocate failed, 
*           else return index(page start)
*/
    unsigned int index, tmp;
    unsigned int cnt, t_cnt;
    unsigned char *res;

    // align operation
    size += ((1 << PAGE_SHIFT) - 1);
    size &= (~((1 << PAGE_SHIFT) - 1));
    // calculate the page number of required size
    cnt = size >> PAGE_SHIFT;

    // first search from last_alloc to the end
    res = find_pages(cnt, bmm.last_alloc + 1, bmm.max_pn, align >> PAGE_SHIFT);
    if (res)
    {
        if (insert_mminfo(&bmm, (unsigned int)res, (unsigned int)res + size - 1, type))
            return res;
    }

    // then search form 0 to last_alloc
    res = find_pages(cnt, 0, bmm.last_alloc, align >> PAGE_SHIFT);
    if (res)
    {
        if (insert_mminfo(&bmm, (unsigned int)res, (unsigned int)res + size - 1, type))
            return res;
    }

    // not found or insert failed, return 0
    return 0;
}

unsigned int split_mminfo(struct bootmm *mm, unsigned int index, unsigned int split_start)
{
    /* get one sequential memory area to be split into two parts
 *  set the former one.end = split_start-1 
 *  set the latter one.start = split_start
 *  return 0 if failed else return 1
 */
    unsigned int start, end;
    unsigned int tmp;

    start = mm->info[index].start_pn;
    end = mm->info[index].end_pn;
    // align operation
    split_start &= (~((1 << PAGE_SHIFT) - 1));

    if ((split_start <= start) || (split_start >= end))
        return 0; // split_start out of range

    if (mm->count == MAX_INFO)
        return 0; // number of segments are reaching max, cannot alloc anymore segments

    // using copy and move, to get a mirror segment of mm->info[index]
    for (tmp = mm->count - 1; tmp >= index; --tmp)
    {
        mm->info[tmp + 1] = mm->info[tmp];
    }
    mm->info[index].end_pn = split_start - 1;
    mm->info[index + 1].start_pn = split_start;
    mm->count++;
    return 1;
}

unsigned int bootmm_free_pages(unsigned int start, unsigned size)
{
/* free pages of size from start in the bootmm
 *  return 0 if failed else return 1
 */

    unsigned int index, cnt;
    // align the size
    size &= ~((1 << PAGE_SHIFT) - 1);
    cnt = size >> PAGE_SHIFT ;
    if (!cnt) return 1;

    start &= ~((1 << PAGE_SHIFT) - 1);
    for (index = 0; index < bmm.count; index++)
    {
        // find the block that cover the input start
        if (bmm.info[index].end_pn < start)
            continue;
        if (bmm.info[index].start_pn > start)
            continue;
        if (start + size - 1 > bmm.info[index].end_pn)
            continue;
        break;
    }
    if (index == bmm.count)
    {
        kernel_printf("  bootmm_free_pages: not alloc space(%x:%x)\n", start, size);
        return 0;
    }
    // find the segment, then split and delete
    set_maps(start >> PAGE_SHIFT, cnt, PAGE_FREE);
    if (bmm.info[index].start_pn == start)
    {
        if (bmm.info[index].end_pn == (start + size - 1))
            remove_mminfo(&bmm, index);
        else
            set_mminfo(&(bmm.info[index]), start + size, bmm.info[index].end_pn, bmm.info[index].type);
    }
    else
    {
        if (bmm.info[index].end_pn == (start + size - 1))
            set_mminfo(&(bmm.info[index]), bmm.info[index].start_pn, start - 1, bmm.info[index].type);
        else
        {
            split_mminfo(&bmm, index, start);
            split_mminfo(&bmm, index + 1, start + size);
            remove_mminfo(&bmm, index + 1);
        }
    }
}