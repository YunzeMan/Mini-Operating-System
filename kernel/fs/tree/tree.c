#include "tree.h"
#include <zjunix/slab.h>
#include <driver/vga.h>
#include <zjunix/utils.h>

void init_treenode(struct filetree * p, char * name)
{
    int i = 0;
    p = (struct filetree *)kmalloc(sizeof(struct filetree));
    p->child = NULL;
    p->parent = NULL;
    do {
        p->name[i] = name[i];
        i++;
    } while(name[i] != '\0' && i < 256);
    p->next = NULL;
    p->before = NULL;
}

void outputDir(struct filetree * tParent)
{
    struct filetree * p = tParent->child;
    while(tParent->child != NULL)
    {
        kernel_printf("%s",p->name);
        p = p->next;
    }
}

void becomeChild(struct filetree * tParent, struct filetree * tChild)
{
    if (tParent->child == NULL)
    {
        tParent->child = tChild;
        tChild->parent = tParent;
    }
    else 
    {
        tChild->next = tParent->child;
        tParent->child->before = tChild;
        tParent->child = tChild;
        tChild->parent = tParent;
    }
}

void FindTree(struct filetree * tParent, char * param)
{
    
}

void DeleteNode(struct filetree * p)
{
    if(p->parent->child == p)
    {
        p->parent->child = p->next;
        p->next->parent = p->parent;
        p->next->before = NULL;
    }
    else
    {
        struct filetree * t = p->parent->child;
        while(t && t->next!=p)
        {
            t = t->next;
        }
        if(t == NULL)
        {
            return;
        }
        else
        {
            t->next = p->next;
        }
    }
}

void print_tree(struct filetree * ft)
{
    kernel_printf("%s", ft->name);
    struct filetree * p;
    struct filetree * pt;
    p = ft;
    while(p->child != NULL) 
    {
        pt = p->child;
        while(pt->next != NULL)
        {
            kernel_printf("%s",pt->name);
        }
        p = p->child;
    }
}

int Empty(struct filetree * ft)
{
    if(ft->child == NULL)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void matching(char * param)
{

}
