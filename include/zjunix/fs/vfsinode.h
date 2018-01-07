#ifndef _ZJUNIX_FS_VFSINODE_H
#define _ZJUNIX_FS_VFSINODE_H

#include<zjunix/type.h>

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

struct buffer_head {
    struct buffer_head *b_next;   /* Hash queue list */
    u32 b_block_number;           /* block number */
    u16 b_size;                   /* block size */
    u16 b_list;                   /* list that buffer appears in the list */
    u32 b_count;                  /* reference times */
    u32 b_state;                  /* buffer state bitmap */
};

struct vfsinode {
    u8 ext[3];                    /* Extension */
    u8 attr;                      /* attribute bits */
    u8 lcase;                     /* Case for base and extension */
    u8 ctime_cs;                  /* Creation time, centiseconds (0-199) */
    u16 ctime;                    /* Creation time */
    u16 cdate;                    /* Creation date */
    u16 adate;                    /* Last access date */
    u16 starthi;                  /* Start cluster (Hight 16 bits) */
    u16 time;                     /* Last modify time */
    u16 date;                     /* Last modify date */
    u16 startlow;                 /* Start cluster (Low 16 bits) */
    u32 size;                     /* file size (in bytes) */
    struct list_head i_hash;      /* The pointer to hash list */
    struct list_head i_list;      /* The pointer to index inode list */
    struct list_head i_dentry;    /* The pointer to dentry list */
    struct list_head i_dirty_buffers;
    struct list_head i_dirty_data_buffers;  /* dirty data buffers */
    u32 ino;                      /* index inode id */
    u32 count;                    /* reference count */
    u32 uid;                      /* user id */
    u32 gid;                      /* group id */
    u32 blockbits;                /* block size (in bits) */
    u32 blocksize;                /* block size (in bytes) */
};

struct vfsinode* raw_vfsinode(struct superblock *sb, u32 ino, struct buffer_head **bh);
struct vfsinode* new_vfsinode(struct superblock *sb);
void delete_vfsinode(struct inode *inode);

#endif // !_ZJUNIX_FS_VFSINODE_H