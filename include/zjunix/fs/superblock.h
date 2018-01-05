#ifndef _ZJUNIX_FS_SUPERBLOCK
#define _ZJUNIX_FS_SUPERBLOCK

#include <zjunix/list.h>

struct superblock {
    struct list_head s_list;           /* 指向超级块链表的指针 */
    u32 s_blocksize;                   /* 以字节为单位的block块大小 */
    u8 s_blocksize_bits;               /* block size大小占用的位数 */ 
    u8 s_dirt;                         /* 脏位 */ 
    u32 s_count;                       /* 被使用的次数 */
    u32 s_magic;                       /* 区别不同的文件系统 */
    struct list_head s_nodes;          /* 所有索引节点inodes连接形成的链表 */
    struct list_head s_dirty;          /* 已经修改的索引节点inode连接形成的链表 */
    struct list_head s_locked_inodes   /* 要进行同步的索引节点形成的链表 */
    struct list_head s_files;          /* 所有的已经打开文件的链表 */
}

#endif