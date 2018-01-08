#include "balloc.h"

/* get group descriptor */
struct myext2_group_desc * myext2_get_group_desc(struct myext2_sb_info * sb, unsigned int block_group, struct buffer_head ** bh)
{
    u32 group_desc;  
    u32 offset;  
    struct myext2_group_desc * desc;    
    /* if the parameter is larger than the number of group, then return NULL */  
    if (block_group >= sb->s_groups_count) {  
        return NULL;  
    }  
    /* right shift such bits equals divide the number of group descriptor*/  
    group_desc = block_group >> sb->s_desc_per_block_bits;  
    /* the offset mean the number of group descriptor in group */  
    offset = block_group & (sb->s_desc_per_block - 1);  
    /* if sb's group descriptor is empty then there is a error with ext2 */  
    if (!sb->s_group_desc[group_desc]) {  
        return NULL;  
    }    
    desc = (struct myext2_group_desc *) sb->s_group_desc[group_desc]->b_data;  
    /* if the parameter bh is not empty then assign group descriptor's buffer_head to bh*/  
    if (bh)  
        *bh = sb->s_group_desc[group_desc];  
    /* return group descriptor */  
    return desc + offset; 
}