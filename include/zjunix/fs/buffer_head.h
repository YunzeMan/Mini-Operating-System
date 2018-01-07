#ifndef _ZJUNIX_FS_BUFFER_HEAD_H
#define _ZJUNIX_FS_BUFFER_HEAD_H

#include <zjunix/type.h>
#include <zjunix/list.h>

struct buffer_head {
    /* First cache line: */
	unsigned long b_state;		/* buffer state bitmap (see above) */
	struct buffer_head *b_this_page;/* circular list of page's buffers */
	//struct page *b_page;		/* the page this bh is mapped to */
	u32 b_count;		/* users using this block */
	u32 b_size;			/* block size */
	u32 b_blocknr;		/* block number */
	char *b_data;			/* pointer to data block */
	//struct block_device *b_bdev;
	//bh_end_io_t *b_end_io;		/* I/O completion */
 	void *b_private;		/* reserved for b_end_io */
	//struct list_head b_assoc_buffers; /* associated with another mapping */
};

enum bh_state {
    BH_Uptodate,    /* if the buffer contains valid data then set to 1 */  
    BH_Dirty,       /* if the buffer is dirty then set to 1 */  
    BH_Lock,        /* if the buffer is blocked then set to 1 */  
    BH_Req,         /* if the buffer is invalid then set to 0 */  
    BH_Mapped,      /* if the buffer has disk then set to 1 */  
    BH_New,         /* if the buffer is new, then set to 1 */  
    BH_Async,       /* if the buffer is synchroned with end_buffer_io_async I/O then set to 1 */  
    BH_Wait_IO,     /* if the buffer has to write back then set to 1 */  
    BH_Launder,     /* if we need to reset this buffer then set to 1 */  
    BH_Attached,    /* 1 if b_inode_buffers is linked into a list */  
    BH_JBD,         /* if link with journal_head then set to 1 */  
    BH_Sync,        /* if the buffer is synchronized then set to 1 */  
    BH_Delay,       /* if the buffer is allocated delay then set to 1 */  

    BH_PrivateStart,/* not a state bit, but the first bit available 
                    * for private allocation by other entities 
                    */  
}

#endif