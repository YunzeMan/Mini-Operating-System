#include "vfs.h"

extern struct vfs* vfsfile;

void initial_vfs() {
    /* Binding */
    vfsfile->fat32_file->find = &fs_find;
    vfsfile->fat32_file->init = &init_fs;
    vfsfile->fat32_file->open = &fs_open;
    vfsfile->fat32_file->close = &fs_close;
    vfsfile->fat32_file->read = &fs_read;
    vfsfile->fat32_file->close = &fs_close;
    vfsfile->fat32_file->fflush = &fs_fflush;
    vfsfile->fat32_file->create = &fs_create;
    vfsfile->fat32_file->mkdir = &fs_mkdir;
    vfsfile->fat32_file->rmdir = &fs_rmdir;
    vfsfile->fat32_file->rm = &fs_rm;
    vfsfile->fat32_file->mv = &fs_mv;
    vfsfile->fat32_file->cp = &fs_cp;
    vfsfile->fat32_file->open_dir = &fs_open_dir;
    vfsfile->fat32_file->read_dir = &fs_read_dir;
    vfsfile->fat32_file->cat = &fs_cat;
}

