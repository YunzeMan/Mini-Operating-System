#ifndef _ZJUNIX_FS_EXT2_H
#define _ZJUNIX_FS_EXT2_H

#include <zjunix/type.h>

struct buffer_head {
    struct buffer_head *b_next;   /* Hash queue list */
    u32 b_block_number;           /* block number */
    u16 b_size;                   /* block size */
    u16 b_list;                   /* list that buffer appears in the list */
    u32 b_count;                  /* reference times */
    u32 b_state;                  /* buffer state bitmap */
};

struct ext2_group_desc {
    u32	bg_block_bitmap;		/* Blocks bitmap block */
	u32	bg_inode_bitmap;		/* Inodes bitmap block */
	u32	bg_inode_table;		    /* Inodes table block */
	u16	bg_free_blocks_count;	/* Free blocks count */
	u16	bg_free_inodes_count;	/* Free inodes count */
	u16	bg_used_dirs_count;	    /* Directories count */
	u16	bg_pad;
	u32	bg_reserved[3];
};

struct ext2_inode {
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
	u32	i_block[15];    /* Pointers to blocks */
	u32	i_generation;	/* File version (for NFS) */
	u32	i_file_acl;	    /* File ACL */
	u32	i_dir_acl;	    /* Directory ACL */
	u32	i_faddr;	    /* Fragment address */
};

struct ext2_super_block {
    u32	s_inodes_count;		  /* Inodes count */
	u32	s_blocks_count;		  /* Blocks count */
	u32	s_r_blocks_count;	  /* Reserved blocks count */
	u32	s_free_blocks_count;  /* Free blocks count */
	u32	s_free_inodes_count;  /* Free inodes count */
	u32	s_first_data_block;	  /* First Data Block */
	u32	s_log_block_size;	  /* Block size */
	u32	s_log_frag_size;	  /* Fragment size */
	u32	s_blocks_per_group;	  /* # Blocks per group */
	u32	s_frags_per_group;	  /* # Fragments per group */
	u32	s_inodes_per_group;	  /* # Inodes per group */
	u32	s_wtime;		      /* Write time */
	u16	s_max_mnt_count;	  /* Maximal mount count */
	u16	s_magic;		      /* Magic signature */
	u16	s_state;		      /* File system state */
	u16	s_def_resuid;		  /* Default uid for reserved blocks */
	u16	s_def_resgid;		  /* Default gid for reserved blocks */
	u32	s_first_ino; 		  /* First non-reserved inode */
	u16 s_inode_size; 		  /* size of inode structure */
	u16	s_block_group_nr; 	  /* block group # of this superblock */
	u32	s_feature_compat; 	  /* compatible feature set */
	u32	s_feature_incompat;   /* incompatible feature set */
	u32	s_feature_ro_compat;  /* readonly-compatible feature set */
	u8	s_uuid[16];		      /* 128-bit uuid for volume */
	u8  s_volume_name[16]; /* volume name */
	u8  s_last_mounted[64];   /* directory where last mounted */
	u32	s_algorithm_usage_bitmap; /* For compression */
	u8	s_prealloc_blocks;	      /* Nr of blocks to try to preallocate*/
	u8	s_prealloc_dir_blocks;	  /* Nr to preallocate for dirs */
};

//struct ext2_dir_entry {
//	u32	inode;			/* Inode number */
//	u16	rec_len;		/* Directory entry length */
//	u16	name_len;		/* Name length */
//	u8  name[16];	    /* File name */
//};

struct ext2_dir_entry_2 {
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
	EXT2_FT_UNKNOWN,   /* unknown */
	EXT2_FT_REG_FILE,  /* normal file */
	EXT2_FT_DIR,       /* directory file */
	EXT2_FT_CHRDEV,    /* char dev file */
	EXT2_FT_BLKDEV,    /* block dev file */
	EXT2_FT_FIFO,      /* named pipe file */
	EXT2_FT_SOCK,      /* socket file */
	EXT2_FT_SYMLINK,   /* link file */
	EXT2_FT_MAX        /* The max number of file type */
};

struct ext2_sb_info {
    u32 s_frag_size;	      /* Size of a fragment in bytes */
	u32 s_frags_per_block;  /* Number of fragments per block */
	u32 s_inodes_per_block; /* Number of inodes per block */
	u32 s_frags_per_group;  /* Number of fragments in a group */
	u32 s_blocks_per_group; /* Number of blocks in a group */
	u32 s_inodes_per_group; /* Number of inodes in a group */
	u32 s_itb_per_group;	  /* Number of inode table blocks per group */
	u32 s_gdb_count;	      /* Number of group descriptor blocks */
	u32 s_desc_per_block;	  /* Number of group descriptors per block */
	u32 s_groups_count;	  /* Number of groups in the fs */
	struct buffer_head * s_sbh;	      /* Buffer containing the super block */
	struct ext2_super_block * s_es;	  /* Pointer to the super block in the buffer */
	struct buffer_head ** s_group_desc;
	u32 s_addr_per_block_bits;
	u32 s_desc_per_block_bits;
	u32 s_inode_size;
	u32 s_first_ino;
	u32 s_dir_count;
};

struct ext2_inode_info {
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

u32 init_ext2();
u32 ext2_find();
u32 ext2_open(FILE *file, unsigned char *filename);
u32 ext2_close(FILE *file);
u32 ext2_read(FILE *file, unsigned char *buf, unsigned long count);
u32 ext2_write(FILE *file, const unsigned char *buf, unsigned long count);
u32 ext2_fflush();
void ext2_lseek(FILE *file, unsigned long new_loc);

/* balloc.c */
extern int ext2_new_block (struct inode *, unsigned long, __u32 *, __u32 *, int *);
extern void ext2_free_blocks (struct inode *, unsigned long, unsigned long);
extern unsigned long ext2_count_free_blocks (struct super_block *);
extern unsigned long ext2_count_dirs (struct super_block *);

/* dir.c */
extern int ext2_make_empty(struct inode *, struct inode *);
extern int ext2_empty_dir (struct inode *);

/* inode.c */
extern void ext2_read_inode (struct inode *);
extern int ext2_write_inode (struct inode *, int);
extern void ext2_put_inode (struct inode *);
extern void ext2_delete_inode (struct inode *);
extern int ext2_sync_inode (struct inode *);
extern void ext2_discard_prealloc (struct inode *);
extern int ext2_get_block(struct inode *, sector_t, struct buffer_head *, int);
extern void ext2_truncate (struct inode *);

/* file.c */

#endif // !_ZJUNIX_FS_EXT2_H