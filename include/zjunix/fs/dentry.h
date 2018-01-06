#ifndef _ZJUNIX_FS_DENTRY_H
#define _ZJUNIX_FS_DENTRY_H

#include <zjunix/type.h>

struct dentry {
    u32 d_mounted;
    u32 d_time;  /* used by d_revalidata */
    u32 d_vfs_flags;
    struct inode *d_inode;          /* Where the name belongs to NULL is negative */
    struct dentry *d_parent;        /* parent directory */
    struct list_head d_hash;        /* look up hash list */
    struct list_head d_lru;         /* d_count = 0 LRU list */
    struct list_head d_child;       /* child of parent list */  
    struct list_head d_subdirs;     /* our children */  
    struct list_head d_alias;       /* inode alias list */    
    //struct dentry_operations  *d_op;  
    struct super_block * d_sb;      /* The root of the dentry tree */    
    void * d_fsdata;                /* fs-specific data */  
    unsigned char d_iname[DNAME_INLINE_LEN]; /* small names */  
}

#endif // !_ZJUNIX_FS_DENTRY_H