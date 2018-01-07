#ifndef _ZJUNIX_FS_VFS_H
#define _ZJUNIX_FS_VFS_H

#include <zjunix/type.h>
#include <zjunix/fs/fat.h>

struct ext2 {

};

struct fat32 {
    struct FILE * file;
    u32 (*find)(FILE *file);
    u32 (*init)();
    u32 (*open)(FILE *file, unsigned char *filename);
    u32 (*close)(FILE *file);
    u32 (*read)(FILE *file, unsigned char *buf, unsigned long count);
    u32 (*write)(FILE *file, const unsigned char *buf, unsigned long count);
    u32 (*fflush)();
    void (*lseek)(FILE *file, unsigned long new_loc);
    u32 (*create)(unsigned char *filename);
    u32 (*mkdir)(unsigned char *filename);
    u32 (*rmdir)(unsigned char *filename);
    u32 (*rm)(unsigned char *filename);
    u32 (*mv)(unsigned char *src, unsigned char *dest);
    u32 (*cp)(unsigned char *src, unsigned char *dest);
    u32 (*open_dir)(FS_FAT_DIR *dir, unsigned char *filename);
    u32 (*read_dir)(FS_FAT_DIR *dir, unsigned char *buf);
    u32 (*cat)(unsigned char * path);
};

struct vfs {
    u8 attr;                      /* attribute bits */
    u8 ctime_cs;                  /* Creation time, centiseconds (0-199) */
    u16 ctime;                    /* Creation time */
    u16 cdate;                    /* Creation date */
    u16 adate;                    /* Last access date */
    u16 time;                     /* Last modify time */
    u16 date;                     /* Last modify date */
    u32 size;                     /* file size (in bytes) */
    u32 count;                    /* reference count */
    u32 uid;                      /* user id */
    u32 gid;                      /* group id */
    struct fat32 * fat32_file;    /* fat32 file system */
    struct ext2 * ext2_file;      /* ext2 file system */
};

#endif // !_ZJUNIX_FS_VFS_H
