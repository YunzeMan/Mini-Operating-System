#ifndef _ZJUNIX_FS_INODE_H
#define _ZJUNIX_FS_INODE_H

#include<zjunix/type.h>

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



#endif // !_ZJUNIX_FS_INODE_H