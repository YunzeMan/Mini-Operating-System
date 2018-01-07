#ifndef _ZJUNIX_FS_VFSINODE_H
#define _ZJUNIX_FS_VFSINODE_H

#include<zjunix/type.h>
#include<zjunix/list.h>

//struct buffer_head {
//    struct buffer_head *b_next;   /* Hash queue list */
//    u32 b_block_number;           /* block number */
//    u16 b_size;                   /* block size */
//    u16 b_list;                   /* list that buffer appears in the list */
//    u32 b_count;                  /* reference times */
//    u32 b_state;                  /* buffer state bitmap */
//};

struct vfsinode {
    u8 attr;                      /* attribute bits */
    u8 lcase;                     /* Case for base and extension */
    u8 ctime_cs;                  /* Creation time, centiseconds (0-199) */
    u16 ctime;                    /* Creation time */
    u16 cdate;                    /* Creation date */
    u16 adate;                    /* Last access date */
    u16 time;                     /* Last modify time */
    u16 date;                     /* Last modify date */
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

//struct vfsinode* raw_vfsinode(struct superblock *sb, u32 ino, struct buffer_head **bh);
//struct vfsinode* new_vfsinode(struct superblock *sb);
//void delete_vfsinode(struct inode *inode);

#endif // !_ZJUNIX_FS_VFSINODE_H