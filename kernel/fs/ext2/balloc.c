#include "balloc.h"
#include <zjunix/fs/buffer_head.h>


/* 获得组描述块 */
struct myext2_group_desc * myext2_get_group_desc(struct myext2_sb_info * sb, unsigned int block_group, struct buffer_head ** bh)
{
    u32 group_desc;  
    u32 offset;  
    struct myext2_group_desc * desc;    
    /*如果参数大于组的数目，说明参数有问题，返回NULL*/  
    if (block_group >= sb->s_groups_count) {  
        return NULL;  
    }  
    /*右移这些位就等于是除以一个块拥有组描述符的数目。*/  
    group_desc = block_group >> sb->s_desc_per_block_bits;  
    /*这里的offset就是在块内的第几个描述符*/  
    offset = block_group & (sb->s_desc_per_block - 1);  
    /*如果为空，就说明ext2出问题了*/  
    if (!sb->s_group_desc[group_desc]) {  
        return NULL;  
    }  
    /*描述符指针指向对应的buffer_head->b_data就是组描述符所在组的第一个组描述符*/  
    desc = (struct myext2_group_desc *) sb->s_group_desc[group_desc]->b_data;  
    /*如果参数bh不为空，就赋值组描述符的buffer_head给bh*/  
    if (bh)  
        *bh = sb->s_group_desc[group_desc];  
    /*返回想要的组描述符*/  
    return desc + offset; 
}