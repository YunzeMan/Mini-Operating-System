#ifndef _EXT2VFS_H
#define _EXT2VFS_H

#include <zjunix/type.h>
#include <zjunix/list.h>

#define INODE_TYPE_GENERIC 0

struct superblk_ops;
struct inode_ops;
struct dentry_ops;
struct file_ops;

struct dentry;
struct inode;

struct inode 
{
	unsigned long i_ino;				// inode number !! uniquely identifies this inode !!
	u32	i_device;								// device number of device holding this inode
	u32 i_mode;									// mode of this inode
	u32 i_user;									// user who owns this inode
	u32 i_group;								// group who owns this inode
	u32 i_creat_time;						// creation time
	u32 i_mod_time;							// modification time
	u32 i_write_time;						// last write time
	struct inode_ops* i_ops;	// inode operations
	struct file_ops* i_fops;	// associated file operations
	unsigned int i_blkbits;	 		// blocksize in bits
	unsigned long i_blocks;	 		// number of blocks in file
	unsigned short i_bytes;  		// number of bytes in last block
	loff_t i_size;					 		// file size in bytes
};

struct superblock 
{
	u32	s_device_id;
	struct inode* s_first_inode;
	struct inode* s_mount_point;
	struct dentry* s_root;
	short s_blocksize;
	short s_blocksize_bits;  // blocksize in bits;
	u32	s_filesystem_type;
	struct superblk_ops* s_ops;
	
};

struct superblk_ops 
{
	int (*read_inode)(struct inode* inode);
	int (*write_inode)(struct inode* inode);
};

struct inode_ops 
{
	int (*create)(struct inode* dir, struct dentry* dentry, u32 mode);
	struct dentry* (*lookup)(struct inode* dir, struct dentry* dentry);
	int (*unlink)(struct inode* dir, struct dentry* dentry);
	int (*mkdir)(struct inode* dir, struct dentry* dentry, u32 mode);
	int (*rmdir)(struct inode* dir, struct dentry* dentry);
	int (*mknod)(struct inode* dir, struct dentry* dentry, u32 mode, u32 rdev);
	int (*rename)(struct inode* old_dir, struct dentry* old_dentry, struct inode* new_dir, struct dentry* new_dentry);	
};

struct dentry 
{
	char* d_name;	
	struct dentry* d_parent;
	struct list_head d_subdirs;
	struct list_head d_siblings;
	struct inode* d_inode;
	int	d_count;
};

struct dentry_ops
{
};

struct ext2file
{	
	u32 f_mode;
	loff_t f_pos;			// current file offset
	struct dentry* f_dentry;
	struct file_ops* f_fops;
	u32 f_flags;
	int f_count;
};

struct dirent 
{	
	struct inode* d_inode;
	char d_name[1];
};

typedef int (*filldir)(void* dirent,
		const char* fname, int fname_len, loff_t pos, u32 inode , u32 file_type);

struct file_ops {
	int (*llseek)(struct ext2file* fil, size_t offset, size_t origin);
	int (*read)(struct ext2file* fil, char* buf, size_t count, size_t* offset);
	int (*write)(struct ext2file* fil, char* buf, size_t count, size_t* offset);
	int (*ioctl)(struct inode* inode, struct ext2file* fil, u32 cmd, u32 arg);
	int (*open)(struct inode* inode, struct ext2file* fil);
	int (*release)(struct inode* inode, struct ext2file* fil);
	int (*readdir)(struct ext2file* file, void* dirent, filldir filldir);
	//device driver operations for blockdrivers (dd_blksize > 0)
	//readblk, writeblk communicate with the hardware
	//in implementations of concrete drivers
	//read and write are programmed generically to use
	//readpage, writepage
	// fil is here for example
	// a file pointing to a dentry pointing to the inode /dev/sda<x>
	// or /dev/sda
	int (*readblk)(struct ext2file* fil, u32 blk_index, char **buf);
	int (*writeblk)(struct ext2file* fil, u32 blk_index, char *buf);
	//readpage and writepage use readblk and writeblk,
	//readpage and writepage are part of the
	//global buffer system
	int (*readpage)(struct ext2file* fil, u32 page_index, char **buf);
	int (*writepage)(struct ext2file* fil, u32 page_index, char *buf);
};



int do_open(char* fname, u32 fmode);

void init_base_files();

struct inode * create_inode(int type, struct inode* buf);
struct ext2file* create_file(int type, struct ext2file* buf);
struct dirent* create_dirent(int type, struct dirent* buf);

struct file_ops* create_file_ops(int type, struct file_ops* buf);

#define INIT_FIXED_LISTS_LEN 32

#define DEV_IDE	8
#define DEV_IDE1 9

extern struct ext2file fixed_file_list[INIT_FIXED_LISTS_LEN];
extern struct dentry fixed_dentry_list[INIT_FIXED_LISTS_LEN];
extern struct inode fixed_inode_list[INIT_FIXED_LISTS_LEN];

#define MAX_NUM_FILE_OPS_TYPES 16

extern struct file_ops file_ops_table[MAX_NUM_FILE_OPS_TYPES];

extern int hdd_fd;


#endif