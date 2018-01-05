#ifndef _ZJUNIX_DENTRY_H
#define _ZJUNIX_DENTRY_H

#include <zjunix/type.h>

struct dentry {
    struct inode *d_inode;
    struct dentry *d_parent;
    struct list_head d_hash;
    struct list_head d_lru;
    struct list_head d_child;       /* child of parent list */  
    struct list_head d_subdirs;     /* our children */  
    struct list_head d_alias;       /* inode alias list */  
    u32 d_mounted;  
    u32 d_time;           /* used by d_revalidate */  
    struct dentry_operations  *d_op;  
    struct super_block * d_sb;      /* The root of the dentry tree */  
    u32 d_vfs_flags;  
    void * d_fsdata;                /* fs-specific data */  
    unsigned char d_iname[DNAME_INLINE_LEN]; /* small names */  
}

#endif // !_ZJUNIX_DENTRY_H