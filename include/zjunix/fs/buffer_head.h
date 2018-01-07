#ifndef _ZJUNIX_FS_BUFFER_HEAD_H
#define _ZJUNIX_FS_BUFFER_HEAD_H

#include <zjunix/type.h>

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

#endif