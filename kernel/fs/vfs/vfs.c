#include "vfs.h"
#include <zjunix/slab.h>

extern struct vfs* vfsfile;

void initial_vfs() {
    /* apply space */
    vfsfile = (struct vfs*)kmalloc(sizeof(struct vfs));
    /* Binding fs_find function */
    vfsfile->fat32_file->find = &fs_find;
    /* Binding init_fs function */
    vfsfile->fat32_file->init = &init_fs;
    /* Binding fs_open function */
    vfsfile->fat32_file->open = &fs_open;
    /* Binding fs_close function */
    vfsfile->fat32_file->close = &fs_close;
    /* Binding fs_read function */
    vfsfile->fat32_file->read = &fs_read;
    /* Binding fs_close function */
    vfsfile->fat32_file->close = &fs_close;
    /* Binding fs_fflush function */
    vfsfile->fat32_file->fflush = &fs_fflush;
    /* Binding fs_create function */
    vfsfile->fat32_file->create = &fs_create;
    /* Binding fs_mkdir function */
    vfsfile->fat32_file->mkdir = &fs_mkdir;
    /* Binding fs_rmdir function */
    vfsfile->fat32_file->rmdir = &fs_rmdir;
    /* Binding fs_rm function */
    vfsfile->fat32_file->rm = &fs_rm;
    /* Binding fs_mv function */
    vfsfile->fat32_file->mv = &fs_mv;
    /* Binding fs_op function */
    vfsfile->fat32_file->cp = &fs_cp;
    /* Binding fs_open_dir function */
    vfsfile->fat32_file->open_dir = &fs_open_dir;
    /* Binding fs_read_dir function */
    vfsfile->fat32_file->read_dir = &fs_read_dir;
    /* Binding fs_cat function */
    vfsfile->fat32_file->cat = &fs_cat;
}

