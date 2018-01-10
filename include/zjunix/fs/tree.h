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

/* the root */
struct filetree * treeparent;

/* initial treenode */
void init_treenode(struct filetree * p, char * name);

/* Output filename in the directory */
void outputDir(struct filetree * tParent);

/* become the child of node */
void becomeChild(struct filetree * tParent, struct filetree * tChild);

/* find tree */
void FindTree(struct filetree * tParent, char * param);

/* Delete node */
void DeleteNode(struct filetree * tTree);

/* Print file tree */
void print_tree(struct filetree * ft);

/* if there is no file */
int Empty(struct filetree * ft);

/* matching file */
void matching(char * param);

#endif // !_ZJUNIX_FS_TREE_H