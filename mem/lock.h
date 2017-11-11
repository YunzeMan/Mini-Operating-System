#ifndef LOCK_H
#define LOCK_H

#include "list.h"

//TODO: implement lock!!!

struct lock_t {
    unsigned int spin;
    struct list_head wait;
};

extern void init_lock(struct lock_t *lock);
extern unsigned int lockup(struct lock_t *lock);
extern unsigned int unlock(struct lock_t *lock);

#endif