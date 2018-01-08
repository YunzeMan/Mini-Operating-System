#ifndef _ZJUNIX_FS_TREE_H
#define _ZJUNIX_FS_TREE_H

#include <zjunix/type.h>

#define MAX_NAME_LEN 256

struct filetree {
    u8 name[256];
    struct filetree * child;
    struct filetree * parent;
    struct filetree * next;
    struct filetree * before;
};

struct filetree * treeparent;

void init_treenode(struct filetree * p, char * name);

void outputDir(struct filetree * tParent);

void becomeChild(struct filetree * tParent, struct filetree * tChild);

void FindTree(struct filetree * tParent, char * param);

void DeleteNode(struct filetree * tTree);

void print_tree(struct filetree * ft);

#endif // !_ZJUNIX_FS_TREE_H