#ifndef _ZJUNIX_FS_TREE_H
#define _ZJUNIX_FS_TREE_H

#include <zjunix/type.h>

#define MAX_NAME_LEN 256

struct filetree { 
    u8 name[256];                 /* File name */
    u16 ctime;                    /* Creation time */
    u16 cdate;                    /* Creation date */
    u32 size;                     /* file size (in bytes) */
    struct filetree * child;      /* The pointer to child */
    struct filetree * parent;     /* The pointer to parent */
    struct filetree * next;       /* The pointer to next brother */
    struct filetree * before;     /* The pointer to before brother */
};

struct filetree * root;   /* The root of tree */
/* initial root */     
void init_filetree();

/* initial treenode */
struct filetree * init_treenode(char * name);

/* Output filename in the directory */
void outputDir(struct filetree * tParent);

/* become the child of node */
void becomeChild(struct filetree * tParent, struct filetree * tChild);

/* find tree */
struct filetree * findNode(char * param);

/* Delete node */
void deleteNode(char * param);

/* Print file tree */
void print_tree(struct filetree * ft);

/* if there is no file */
int Empty(struct filetree * ft);

/* matching file */
int matching(struct filetree * p, char * param);

#endif // !_ZJUNIX_FS_TREE_H