#include "tree.h"
#include <zjunix/slab.h>
#include <driver/vga.h>
#include <zjunix/utils.h>

void init_filetree()
{
    int i;
    root = (struct filetree *)kmalloc(sizeof(struct filetree));
    root->child = NULL;
    root->parent = NULL;
    for(i = 0; i < 256; i++)
    {
        root->name[i] = 0;
    }
    root->name[0] = '/';
    root->next = NULL;
    root->before = NULL;
}

struct filetree * init_treenode(char * name)
{
    int i;
    struct filetree * p = (struct filetree *)kmalloc(sizeof(struct filetree));
    p->child = NULL;
    p->parent = NULL;
    p->next = NULL;
    p->before = NULL;
    for(i = 0; i < 256; i++)
    {
        p->name[i] = 0;
    }
    i = 0;
    do {
        p->name[i] = name[i];
        i++;
    } while(name[i] != '\0' && i < 256);
    return p;
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

struct filetree * findNode (char * param)
{
    struct filetree * p;
    struct filetree * c;
    p = root;
    while(p != NULL)
    {
        c = p;
        if(matching(c,param))
        {
            return c;
        }
        else
        {
            while(c->next != NULL)
            {
                c = c->next;
                if(matching(c,param))
                {
                    return c;
                }
            }
            p = p->child;
        }
    }
    return NULL;
}

void deleteNode(char * param)
{
    struct filetree * p;
    struct filetree * t;
    struct filetree * t1;
    p = findNode(param);
    kernel_printf("%s\n",p->name);
    if(p->parent == NULL)
    {
        kernel_printf("parent is NULL\n");
        t = p->before;
        if(t != NULL)
        {
            t->next = p->next;
            p->before = NULL;
            p->next = NULL;
            kfree(p);
        }
        else
        {
            kfree(p);
        }
    }
    else
    {
        kernel_printf("parent is not NULL\n");
        t = p->parent;
        t1 = p->next;
        if(t1 != NULL)
        {
            t->child = t1;
            t1->parent = t;
            p->next = NULL;
            t1->before = NULL;
        }
        else
        {
            t->child = NULL;
            p->parent = NULL;
            p->before = NULL;
            p->next = NULL;
        }
    }
    /*if(p->parent->child == p)
    {
        p->parent->child = p->next;
        p->next->parent = p->parent;
        p->next->before = NULL;
    }
    else
    {
        t = p->parent->child;
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
    }*/
}

void print_tree(struct filetree * ft)
{
    struct filetree * p;
    struct filetree * c;
    p = ft;
    while(p != NULL)
    {
        c = p;
        kernel_printf("    %s\n", c->name);
        while(c->next != NULL)
        {
            c = c->next;
            kernel_printf("    %s\n", c->name);
        }
        p=p->child;
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

int matching(struct filetree * p, char * param)
{
    int i;
    //len = strlen(p->name);
    unsigned int len = 0;
    while (p->name[len])
        ++len;
    for(i=0; i<=len; i++)
    {
        if(p->name[i] == param[i])
        {
        }
        else
        {
            break;
        }
    }
    if(i<=len)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}
