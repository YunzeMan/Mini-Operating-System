#ifndef _ZJUNIX_FS_MYEXT2_H
#define _ZJUNIX_FS_MYEXT2_H

#include <zjunix/fs/ext2vfs.h>
#include <zjunix/fs/buffer_head.h>
#include <zjunix/type.h>

#define SIZE_SB 1024

struct superblock_ext2{
    unsigned int s_inodes_count;  // total inode count
    unsigned int s_free_inodes_count; //free inode count
    unsigned int s_blocks_count;  //total data block count
    unsigned int s_free_blocks_count; //free data block count

    unsigned int s_first_data_block;  // first data block
    unsigned int s_log_block_size; //block size 
    unsigned int s_log_frag_size; //fragment size

    unsigned int s_blocks_per_group; //blocks per group
    unsigned int s_frags_per_group; //fragments per group
    unsigned int s_inodes_per_group;  //inodes per group
    
    unsigned int s_mtime;  // mount time
    unsigned int s_wtime;  // write time
    unsigned short s_mnt_count;  // mount count
    unsigned short s_max_mnt_count; //max mount count
	unsigned short s_magic;
};

struct bg_desc_ext2{
    unsigned int bg_block_bitmap;  //block bitmap
    unsigned int bg_inode_bitmap;  // inode bitmap
    unsigned int bg_inode_table;  // inode table block

    unsigned short bg_free_block_count;  // free blocks count
    unsigned short bg_free_inodes_count;  // free inodes count
    unsigned short bg_used_dirs_count;  // directories count

};


struct inode_ext2{
    unsigned short i_mode;  // file mode
    unsigned int i_size;   // file size
    unsigned int i_atime;   // file last read time
    unsigned int i_ctime;  // file create time
    unsigned int i_mtime;  // file last modify time
    unsigned int i_dtime;  // file delete time

    unsigned short i_links_count;  // count of links to blocks 
    unsigned int i_blocks;
    unsigned int i_flags;   //file flag
    unsigned int i_block[15];  // pointers to data block

    unsigned int i_file_acl;
    unsigned int i_dir_acl;

};

struct dir_entry_ext2{
    unsigned int inode;
    unsigned short rec_len;
    unsigned char name_len;
    unsigned char file_type;
};

//struct file_ext2 ;

struct blk_iterator
{
	int mode;
	unsigned int index[4];
	char* blks[4];
	int blk_valid[4];
	unsigned int blks_blk[4];

	struct file_ext2* file;
	struct inode_ext2* pinode;
	unsigned int inode_pos;

	int code;
	struct file *dev_file;
	unsigned int offset;

};

// TODO: file FROM VFS
struct file_ext2{
    struct file* dev_file;
    struct superblock_ext2* sb;
    struct inode_ext2* pinode;
    
    unsigned int inode_index;
    unsigned int offset_inode;

    struct bg_desc_ext2* pbgd;
    unsigned int pbgd_index;

    struct blk_iterator* it_lpos;
    struct blk_iterator* it_end; 
};

extern struct superblock_ext2* gsb_ext2;

extern unsigned g_blocks_per_group_ext2;

extern struct bg_desc_ext2* gbgd_ext2;

int init_superblock_ext2(struct file* dev_file);

int init_ext2_system(struct file* dev_file);

int print_sb_ext2(struct superblock_ext2* sb);
int print_bgdesc_ext2(struct bg_desc_ext2* pbgdesc);
int print_inode_ext2(struct inode_ext2* pinode);

int read_inode_ext2(struct file_ext2* file, u32 inode_index);
int read_bgdesc_ext2(struct file* dev_file, struct bg_desc_ext2* pbgd, u32 bg_index);

int write_inode_ext2(struct file_ext2* filp);

inline u32 get_first_data_block_pos_in_block_group_bitmap(u32 bgd_index,
		struct superblock_ext2* sb, struct bg_desc_ext2* pbgd);

inline u32 get_last_data_block_pos_in_block_group_bitmap(u32 bgd_index,
		struct superblock_ext2 *sb, struct bg_desc_ext2 *pbgd);
inline u32 get_first_data_block_in_block_group(struct bg_desc_ext2* pbgd, struct superblock_ext2* sb);

int readdir_ext2(struct file_ext2* file, struct dir_entry_ext2* dir_entry,
		char* namebuf, u32 *dir_offset);
int read_file_ext2(struct file_ext2* file, char* buf, u32 counta, u32 offseta);
int write_file_ext2(struct file_ext2* file, char* buf, u32 counta, u32 offseta);

int parse_path_ext2(struct file_ext2* file_pwd, u32 mode, char* path,
		struct file_ext2 *file, char* last_fname);

int get_indirect_blocks(u32 offset, u32* index_arr, u32 *mode);
int test_get_indirect_blocks();

int create_file_ext2(struct file_ext2* filp_parent_dir,
		struct file_ext2 *filp_new_file, char* fname, u16 imode, u32 iflags);

int create_directory_ext2(struct file_ext2* filp_parent_dir,
		struct file_ext2 *filp_new_dir, char* fname, u16 imode, u32 iflags);

int delete_directory_ext2(struct file_ext2* filp_dir, char* fname);

int unlink_file_ext2(struct file_ext2* filp_dir, char* fname);

int display_directory_ext2(struct file_ext2 *file);
int display_regular_file_ext2(struct file_ext2 *file);
int display_inode_ext2(struct file_ext2 *file);

int read_from_dev(struct file* dev_file, char* buf, u32 count, u32 offset);
int write_to_dev(struct file* dev_file, char* buf, u32 count, u32 offset);

int blk_iterator_init(struct blk_iterator *it, struct file_ext2* file,
		u32 offset);
int blk_iterator_destroy(struct blk_iterator* it);

int blk_iterator_next(struct blk_iterator *it, u32 offset_new);

u32 blk_iterator_get_blk_index(struct blk_iterator *it);

struct inode_ext2* alloc_inode_ext2();
struct bg_desc_ext2* alloc_bg_desc_ext2();
struct blk_iterator* alloc_blk_iterator();
struct superblock_ext2* alloc_superblock_ext2();
struct file_ext2* alloc_file_ext2();

int copy_file_ext2(struct file_ext2* fdest, struct file_ext2* fsrc);
int destroy_file_ext2(struct file_ext2* fdestr);

void init_file_ext2(struct file_ext2* filp, struct file* dev_file, struct superblock_ext2* sb);


// randomized testing ext2 file writes

int test_ext2_write(struct file* dev_file, struct superblock_ext2* sb, int no_runs);

#endif