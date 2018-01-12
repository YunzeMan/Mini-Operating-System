#ifndef _ZJUNIX_SEM_H
#define _ZJUNIX_SEM_H

#define SEM_NAME_LENGTH 20

/* Essential Struct sem_t_
 * 
 * The struct sem_t_ save the state of a semaphore.
 *
 * With this struct we are able to control process mutual exclusion. A value 0 suggest the process is blocked,
 * while a value 1 indicates the space is useable.
 * 
 * Note: Queue struct is a self-maintained queue.
 */
typedef struct Queue  
{  
    struct task_struct* point;  
    struct Queue *next;  
} queue;  

typedef struct sem_t_  
{
    queue *head;  
    queue *tail; 
    char name[SEM_NAME_LENGTH];  
    int value;
} sem_t;

void init_sem();
sem_t* sem_open(const char *name, unsigned int value);
int sem_post(sem_t *sem);
int sem_wait(sem_t *sem);
int sem_unlink(const char *name);

#endif  // !_ZJUNIX_SEM_H

