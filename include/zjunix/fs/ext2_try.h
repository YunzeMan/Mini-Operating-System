#ifndef _ZJUNIX_FS_EXT2_TRY_H
#define _ZJUNIX_FS_EXT2_TRY_H

#include <zjunix/type.h>

/* ext2_fs.h */
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
	u16	i_mode;			/* File mode */
	u16	i_uid;			/* Low 16 bits of Owner Uid */
	u32	i_size;			/* Size in bytes */
	u32	i_atime;		/* Access time */
	u32	i_ctime;		/* Creation time */
	u32	i_mtime;		/* Modification time */
	u32	i_dtime;		/* Deletion Time */
	u16	i_gid;			/* Low 16 bits of Group Id */
	u16	i_links_count;	/* Links count */
	u32	i_blocks;		/* Blocks count */
	u32	i_flags;		/* File flags */
	u32	i_block[15];	/* Pointers to blocks */
	u32	i_generation;	/* File version (for NFS) */
	u32	i_file_acl;		/* File ACL */
	u32	i_dir_acl;		/* Directory ACL */
	u32	i_faddr;		/* Fragment address */
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
	u16	s_errors;		      /* Behaviour when detecting errors */
	u16	s_minor_rev_level; 	  /* minor revision level */
	u32	s_lastcheck;		  /* time of last check */
	u32	s_checkinterval;	  /* max. time between checks */
	u32	s_creator_os;		  /* OS */
	u32	s_rev_level;		  /* Revision level */
	u16	s_def_resuid;		  /* Default uid for reserved blocks */
	u16	s_def_resgid;		  /* Default gid for reserved blocks */
	/*
	 * These fields are for EXT2_DYNAMIC_REV superblocks only.
	 *
	 * Note: the difference between the compatible feature set and
	 * the incompatible feature set is that if there is a bit set
	 * in the incompatible feature set that the kernel doesn't
	 * know about, it should refuse to mount the filesystem.
	 * 
	 * e2fsck's requirements are more strict; if it doesn't know
	 * about a feature in either the compatible or incompatible
	 * feature set, it must abort and not try to meddle with
	 * things it doesn't understand...
	 */
	u32	s_first_ino; 		/* First non-reserved inode */
	u16   s_inode_size; 		/* size of inode structure */
	u16	s_block_group_nr; 	/* block group # of this superblock */
	u32	s_feature_compat; 	/* compatible feature set */
	u32	s_feature_incompat; 	/* incompatible feature set */
	u32	s_feature_ro_compat; 	/* readonly-compatible feature set */
	u8	s_uuid[16];		/* 128-bit uuid for volume */
	char	s_volume_name[16]; 	/* volume name */
	char	s_last_mounted[64]; 	/* directory where last mounted */
	u32	s_algorithm_usage_bitmap; /* For compression */
	/*
	 * Performance hints.  Directory preallocation should only
	 * happen if the EXT2_COMPAT_PREALLOC flag is on.
	 */
	u8	s_prealloc_blocks;	/* Nr of blocks to try to preallocate*/
	u8	s_prealloc_dir_blocks;	/* Nr to preallocate for dirs */
	u16	s_padding1;
	/*
	 * Journaling support valid if EXT3_FEATURE_COMPAT_HAS_JOURNAL set.
	 */
	u8	s_journal_uuid[16];	/* uuid of journal superblock */
	u32	s_journal_inum;		/* inode number of journal file */
	u32	s_journal_dev;		/* device number of journal file */
	u32	s_last_orphan;		/* start of list of inodes to delete */
	u32	s_hash_seed[4];		/* HTREE hash seed */
	u8	s_def_hash_version;	/* Default hash version to use */
	u8	s_reserved_char_pad;
	u16	s_reserved_word_pad;
	u32	s_default_mount_opts;
 	u32	s_first_meta_bg; 	/* First metablock block group */
    u32	s_reserved[190];	/* Padding to the end of the block */
};

struct ext2_dir_entry {
	u32	inode;			/* Inode number */
	u16	rec_len;		/* Directory entry length */
	u16	name_len;		/* Name length */
	char name[256];	    /* File name */
};

struct ext2_dir_entry_2 {
	u32	inode;			/* Inode number */
	u16	rec_len;		/* Directory entry length */
	u8	name_len;		/* Name length */
	u8	file_type;
	char name[256];	    /* File name */
};

/*
 * Ext2 directory file types.  Only the low 3 bits are used.  The
 * other bits are reserved for now.
 */
enum {
	EXT2_FT_UNKNOWN,
	EXT2_FT_REG_FILE,
	EXT2_FT_DIR,
	EXT2_FT_CHRDEV,
	EXT2_FT_BLKDEV,
	EXT2_FT_FIFO,
	EXT2_FT_SOCK,
	EXT2_FT_SYMLINK,
	EXT2_FT_MAX
};

/* ext2_fs_sb.h */
struct ext2_sb_info {
	unsigned long s_frag_size;	      /* Size of a fragment in bytes */
	unsigned long s_frags_per_block;  /* Number of fragments per block */
	unsigned long s_inodes_per_block; /* Number of inodes per block */
	unsigned long s_frags_per_group;  /* Number of fragments in a group */
	unsigned long s_blocks_per_group; /* Number of blocks in a group */
	unsigned long s_inodes_per_group; /* Number of inodes in a group */
	unsigned long s_itb_per_group;	  /* Number of inode table blocks per group */
	unsigned long s_gdb_count;	      /* Number of group descriptor blocks */
	unsigned long s_desc_per_block;	  /* Number of group descriptors per block */
	unsigned long s_groups_count;	  /* Number of groups in the fs */
	struct buffer_head * s_sbh;	      /* Buffer containing the super block */
	struct ext2_super_block * s_es;	  /* Pointer to the super block in the buffer */
	struct buffer_head ** s_group_desc;
	unsigned long  s_mount_opt;
	unsigned short s_mount_state;
	unsigned short s_pad;
	int s_addr_per_block_bits;
	int s_desc_per_block_bits;
	int s_inode_size;
	int s_first_ino;
	spinlock_t s_next_gen_lock;
	u32 s_next_generation;
	unsigned long s_dir_count;
};

/* fs.h */
struct inode {
	struct hlist_node	i_hash;
	struct list_head	i_list;
	struct list_head	i_sb_list;
	struct list_head	i_dentry;
	unsigned long		i_ino;
	atomic_t		i_count;
	umode_t			i_mode;
	unsigned int		i_nlink;
	uid_t			i_uid;
	gid_t			i_gid;
	dev_t			i_rdev;
	loff_t			i_size;
	struct timespec		i_atime;
	struct timespec		i_mtime;
	struct timespec		i_ctime;
	unsigned int		i_blkbits;
	unsigned long		i_blksize;
	unsigned long		i_version;
	unsigned long		i_blocks;
	unsigned short      i_bytes;
	spinlock_t		i_lock;	/* i_blocks, i_bytes, maybe i_size */
	struct inode_operations	*i_op;
	struct file_operations	*i_fop;	/* former ->i_op->default_file_ops */
	struct super_block	*i_sb;
	struct address_space	*i_mapping;
	struct address_space	i_data;
	unsigned long		i_state;
};

/* dcache.h */
struct dentry {
	atomic_t d_count;
	unsigned int d_flags;		/* protected by d_lock */
	spinlock_t d_lock;		/* per dentry lock */
	struct inode *d_inode;		/* Where the name belongs to - NULL is
					 * negative */
	/*
	 * The next three fields are touched by __d_lookup.  Place them here
	 * so they all fit in a cache line.
	 */
	struct hlist_node d_hash;	/* lookup hash list */
	struct dentry *d_parent;	/* parent directory */
	struct qstr d_name;

	struct list_head d_lru;		/* LRU list */
	/*
	 * d_child and d_rcu can share memory
	 */
	union {
		struct list_head d_child;	/* child of parent list */
	 	struct rcu_head d_rcu;
	} d_u;
	struct list_head d_subdirs;	/* our children */
	struct list_head d_alias;	/* inode alias list */
	unsigned long d_time;		/* used by d_revalidate */
	struct dentry_operations *d_op;
	struct super_block *d_sb;	/* The root of the dentry tree */
	void *d_fsdata;			/* fs-specific data */
	int d_mounted;
	unsigned char d_iname[256];	/* small names */
};

struct file {
	/*
	 * fu_list becomes invalid after file_free is called and queued via
	 * fu_rcuhead for RCU freeing
	 */
	union {
		struct list_head	fu_list;
	} f_u;
	struct file_operations	*f_op;
	atomic_t		f_count;
	unsigned int 		f_flags;
	mode_t			f_mode;
	loff_t			f_pos;
	unsigned int		f_uid, f_gid;

	unsigned long		f_version;
	void			*f_security;

	/* needed for tty driver, and maybe others */
	void			*private_data;
};

struct file_operations {
    struct module *owner;
	u32 (*llseek) (struct file *, u32, u32);
	u32 (*read) (struct file *, char __user *, u32, u32 *);
	u32 (*write) (struct file *, const char __user *, u32, u32 *);
	u32 (*readdir) (struct file *, void *, u32);
	u32 (*open) (struct inode *, struct file *);
};

struct inode_operations {
	int (*create) (struct inode *,struct dentry *,int, struct nameidata *);
	int (*link) (struct dentry *,struct inode *,struct dentry *);
	int (*unlink) (struct inode *,struct dentry *);
	int (*symlink) (struct inode *,struct dentry *,const char *);
	int (*mkdir) (struct inode *,struct dentry *,int);
	int (*rmdir) (struct inode *,struct dentry *);
	int (*mknod) (struct inode *,struct dentry *,int,dev_t);
	int (*rename) (struct inode *, struct dentry *, struct inode *, struct dentry *);
};

/* ext2.h */
/* The information of ext2 in memory */
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
	struct inode vfs_inode;
};

/* balloc.c */
extern int ext2_bg_has_super(struct super_block *sb, int group);
extern unsigned long ext2_bg_num_gdb(struct super_block *sb, int group);
extern int ext2_new_block (struct inode *, unsigned long, __u32 *, __u32 *, int *);
extern void ext2_free_blocks (struct inode *, unsigned long, unsigned long);
extern unsigned long ext2_count_free_blocks (struct super_block *);
extern unsigned long ext2_count_dirs (struct super_block *);
extern void ext2_check_blocks_bitmap (struct super_block *);
extern struct ext2_group_desc * ext2_get_group_desc(struct super_block * sb, unsigned int block_group, struct buffer_head ** bh);

/* dir.c */
extern int ext2_add_link (struct dentry *, struct inode *);
extern ino_t ext2_inode_by_name(struct inode *, struct dentry *);
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
extern int ext2_setattr (struct dentry *, struct iattr *);
extern void ext2_set_inode_flags(struct inode *inode);

/* dir.c */
extern struct file_operations ext2_dir_operations;

/* file.c */
extern struct inode_operations ext2_file_inode_operations;
extern struct file_operations ext2_file_operations;

/* inode.c */
extern void ext2_read_inode (struct inode *);
extern int ext2_write_inode (struct inode *, int);
extern void ext2_put_inode (struct inode *);
extern void ext2_delete_inode (struct inode *);
extern int ext2_sync_inode (struct inode *);
extern void ext2_discard_prealloc (struct inode *);

/* ialloc.c */
extern struct inode * ext2_new_inode (struct inode *, int);
extern void ext2_free_inode (struct inode *);

/* namei.c */
extern struct inode_operations ext2_dir_inode_operations;
extern struct inode_operations ext2_special_inode_operations;

#endif // ! _ZJUNIX_FS_EXT2_H