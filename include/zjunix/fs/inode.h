#ifndef _ZJUNIX_FS_INODE_H
#define _ZJUNIX_FS_INODE_H

#include<zjunix/type.h>

enum bh_state {
    BH_Uptodate,    /* 如果缓冲区包含有效数据则置1 */  
    BH_Dirty,       /* 如果buffer脏(存在数据被修改情况)，那么置1 */  
    BH_Lock,        /* 如果缓冲区被锁定，那么就置1 */  
    BH_Req,         /* 如果缓冲区无效就置0 */  
    BH_Mapped,      /* 如果缓冲区有一个磁盘映射就置1 */  
    BH_New,         /* 如果缓冲区是新的，而且没有被写出去，那么置1 */  
    BH_Async,       /* 如果缓冲区是进行end_buffer_io_async I/O 同步则置1 */  
    BH_Wait_IO,     /* 如果要将这个buffer写回，那么置1 */  
    BH_Launder,     /* 如果需要重置这个buffer，那么置1 */  
    BH_Attached,    /* 1 if b_inode_buffers is linked into a list */  
    BH_JBD,         /* 如果和 journal_head 关联置1 */  
    BH_Sync,        /* 如果buffer是同步读取置1 */  
    BH_Delay,       /* 如果buffer空间是延迟分配置1 */  

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
}

struct inode {
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
}

struct inode* raw_inode(struct superblock *sb, u32 ino, struct buffer_head **bh);
struct inode* new_inode(struct superblock *sb);
void delete_inode(struct inode *inode);

#endif // !_ZJUNIX_FS_INODE_H