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
    struct list_head i_hash;      /* 指向hash链表的指针 */
    struct list_head i_list;      /* 指向索引节点的链表指针 */
    struct list_head i_dentry;    /* 指向目录项链表指针*/
    struct list_head i_dirty_buffers;
    struct list_head i_dirty_data_buffers; /* 脏数据缓冲区 */
    u32 i_ino;                    /* 索引节点号 */
    u32 i_count                   /* 引用计数 */

}



#endif // !_ZJUNIX_FS_INODE_H