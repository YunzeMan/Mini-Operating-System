#ifndef _ZJUNIX_FS_MYEXT2_H
#define _ZJUNIX_FS_MYEXT2_H

#include <zjunix/fs/buffer_head.h>
#include <zjunix/type.h>

#define EXT2_N_BLOCKS 15

/* 组描述 */
struct myext2_group_desc
{
	u32	bg_block_bitmap;		/* Blocks bitmap block */
	u32	bg_inode_bitmap;		/* Inodes bitmap block */
	u32	bg_inode_table;		    /* Inodes table block */
	u16	bg_free_blocks_count;	/* Free blocks count */
	u16	bg_free_inodes_count;	/* Free inodes count */
	u16	bg_used_dirs_count;	    /* Directories count */
	u16	bg_pad;
	u32	bg_reserved[3];
};

/* 索引节点 */
struct myext2_inode {
	u16	i_mode;		    /* File mode */
	u16	i_uid;		    /* Low 16 bits of Owner Uid */
	u32	i_size;		    /* Size in bytes */
	u32	i_atime;	    /* Access time */
	u32	i_ctime;	    /* Creation time */
	u32	i_mtime;	    /* Modification time */
	u32	i_dtime;	    /* Deletion Time */
	u16	i_gid;		    /* Low 16 bits of Group Id */
	u16	i_links_count;	/* Links count */
	u32	i_blocks;	    /* Blocks count */
	u32	i_flags;	    /* File flags */
	u32	i_block[EXT2_N_BLOCKS];/* Pointers to blocks */
	u32	i_generation;	/* File version (for NFS) */
	u32	i_file_acl;	    /* File ACL */
	u32	i_dir_acl;	    /* Directory ACL */
	u32	i_faddr;	    /* Fragment address */
};

/* 超级块 */
struct myext2_super_block {
	u32	s_inodes_count;		    /* Inodes count */
	u32	s_blocks_count;		    /* Blocks count */
	u32	s_r_blocks_count;	    /* Reserved blocks count */
	u32	s_free_blocks_count;	/* Free blocks count */
	u32	s_free_inodes_count;	/* Free inodes count */
	u32	s_first_data_block;	    /* First Data Block */
	u32	s_log_block_size;	    /* Block size */
	u32	s_log_frag_size;	    /* Fragment size */
    u32	s_blocks_per_group;	    /* # Blocks per group */
	u32	s_frags_per_group;	    /* # Fragments per group */
	u32	s_inodes_per_group;	    /* # Inodes per group */
	u32	s_mtime;		        /* Mount time */
	u32	s_wtime;		        /* Write time */
	u16	s_mnt_count;		    /* Mount count */
	u16	s_max_mnt_count;	    /* Maximal mount count */
	u16	s_magic;		        /* Magic signature */
	u16	s_state;		        /* File system state */
	u32	s_first_ino; 		    /* First non-reserved inode */
	u16 s_inode_size; 		    /* size of inode structure */
	u16	s_block_group_nr; 	    /* block group # of this superblock */
	u32	s_feature_compat; 	    /* compatible feature set */
	u32	s_feature_incompat; 	/* incompatible feature set */
	u32	s_feature_ro_compat; 	/* readonly-compatible feature set */
	u8	s_uuid[16];		        /* 128-bit uuid for volume */
	char	s_volume_name[16]; 	/* volume name */
	char	s_last_mounted[64]; /* directory where last mounted */
	u32	s_reserved[190];	    /* Padding to the end of the block */
};

/* 目录结构 */
struct myext2_dir_entry_2 {
	u32	inode;			/* Inode number */
	u16	rec_len;		/* Directory entry length */
	u8	name_len;		/* Name length */
	u8	file_type;
	u8  name[16];	    /* File name */
};

/*
 * Ext2 directory file types.  Only the low 3 bits are used.  The
 * other bits are reserved for now.
 */
enum {
	MYEXT2_FT_UNKNOWN,   /* unknown */
	MYEXT2_FT_REG_FILE,  /* normal file */
	MYEXT2_FT_DIR,       /* directory file */
	MYEXT2_FT_CHRDEV,    /* char dev file */
	MYEXT2_FT_BLKDEV,    /* block dev file */
	MYEXT2_FT_FIFO,      /* named pipe file */
	MYEXT2_FT_SOCK,      /* socket file */
	MYEXT2_FT_SYMLINK,   /* link file */
	MYEXT2_FT_MAX        /* The max number of file type */
};

/* The information of ext2 superblock in memory */
struct myext2_sb_info {
    u32 s_frag_size;	      /* Size of a fragment in bytes */
	u32 s_frags_per_block;    /* Number of fragments per block */
	u32 s_inodes_per_block;   /* Number of inodes per block */
	u32 s_frags_per_group;    /* Number of fragments in a group */
	u32 s_blocks_per_group;   /* Number of blocks in a group */
	u32 s_inodes_per_group;   /* Number of inodes in a group */
	u32 s_itb_per_group;	  /* Number of inode table blocks per group */
	u32 s_gdb_count;	      /* Number of group descriptor blocks */
	u32 s_desc_per_block;	  /* Number of group descriptors per block */
	u32 s_groups_count;	      /* Number of groups in the fs */
	struct buffer_head * s_sbh;	          /* Buffer containing the super block */
	struct myext2_super_block * s_es;	  /* Pointer to the super block in the buffer */
	struct buffer_head ** s_group_desc;
	u32 s_addr_per_block_bits;
	u32 s_desc_per_block_bits;
	u32 s_inode_size;
	u32 s_first_ino;
	u32 s_dir_count;
};

/* The information of ext2 inode in memory */
struct myext2_inode_info {
    u32	i_data[15];
	u32	i_flags;
	u32	i_faddr;
	u8	i_frag_no;
	u8	i_frag_size;
	u16	i_state;
	u32	i_file_acl;
	u32	i_dir_acl;
	u32	i_dtime;
	u32	i_block_group;
	u32	i_next_alloc_block;
	u32	i_next_alloc_goal;
	u32	i_prealloc_block;
	u32	i_prealloc_count;
	u32	i_dir_start_lookup;
	//struct inode vfs_inode;
};

u32 init_myext2();
u32 myext2_find();
u32 myext2_open(FILE *file, unsigned char *filename);
u32 myext2_close(FILE *file);
u32 myext2_read(FILE *file, unsigned char *buf, unsigned long count);
u32 myext2_write(FILE *file, const unsigned char *buf, unsigned long count);
u32 myext2_fflush();
void myext2_lseek(FILE *file, unsigned long new_loc);

/* balloc.c */
extern int myext2_new_block (struct inode *, unsigned long, __u32 *, __u32 *, int *);
extern void myext2_free_blocks (struct inode *, unsigned long, unsigned long);
extern u32 myext2_count_free_blocks (struct super_block *);
extern u32 myext2_count_dirs (struct super_block *);
extern struct myext2_group_desc * myext2_get_group_desc(struct myext2_sb_info * sb, unsigned int block_group, struct buffer_head ** bh);

/* dir.c */
extern int myext2_make_empty(struct inode *, struct inode *);
extern int myext2_empty_dir (struct inode *);

/* inode.c */
extern void myext2_read_inode (struct inode *);
extern int myext2_write_inode (struct inode *, int);
extern void myext2_put_inode (struct inode *);
extern void myext2_delete_inode (struct inode *);
extern int myext2_sync_inode (struct inode *);
extern void myext2_discard_prealloc (struct inode *);
extern int myext2_get_block(struct inode *, sector_t, struct buffer_head *, int);
extern void myext2_truncate (struct inode *);

/* file.c */

#endif //! _ZJUNIX_FS_MYEXT2_H