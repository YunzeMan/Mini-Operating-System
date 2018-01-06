#ifndef _ZJUNIX_FS_SUPERBLOCK_H
#define _ZJUNIX_FS_SUPERBLOCK_H

#include <zjunix/list.h>
#include <zjunix/type.h>
#include <zjunix/fs/magic.h>

#define SUPER_BLOCK_SIZE 4096
#define SUPER_BLOCK_LOG_SIZE 12 // 2^12 = 4096
#define SUPER_BLOCK_MAX_LINK 65535 //

struct superblock {
    u8 s_blocksize_bits;               /* block size (in bits) */ 
    u8 s_dirt;                         /* dirty bit */ 
    u32 s_count;                       /* used times */
    u32 s_magic;                       /* distinguish different file system */
    u32 s_blocksize;                   /* block size (in bytes) */
    struct list_head s_list;           /* The pointer to super block list */
    struct list_head s_nodes;          /* The pointer to all inodes list */
    struct list_head s_dirty;          /* The pointer to modified inodes list */
    struct list_head s_locked_inodes   /* all inodes synch list */
    struct list_head s_files;          /* all opened files list */
}

#endif // ! _ZJUNIX_FS_SUPERBLOCK_H