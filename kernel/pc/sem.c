#include "sem.h" 
 
#include <driver/vga.h>
#include <intr.h>
#include <zjunix/syscall.h>
#include <zjunix/utils.h>
#include <zjunix/pc.h>
#include <zjunix/time.h>
#include <zjunix/slab.h>
#include <zjunix/buddy.h>
  
int sem_num;                   //The number of semaphores
sem_t* semaphore[64];        //Save the value of semaphores 


/* Initiate semaphore part 
 * 
 *Function init_pc initiates the semaphore functions of the OS. It set number of semaphore to 0. 
 *Further operations can be added.
 *@No parameters
 *@No return value
 */
void init_sem()
{
    sem_num = 0;
}

/* Open a sem struct
 * 
 *Function pc_schedule schedules the processes, it choose the process from a multi-level queue, 
 *processes with higher priority is scheduled first. The priority is dynamic.
 *@param status input of system calls and interrupts, the state of CPU
 *@param cause input of the system calls and interrupts, the cause of the interruption
 *@param pt_context input of the system calls and interrupts, the context of the current PC
 *@No return value
 */  
sem_t* sem_open(const char *name, unsigned int value)  
{  
    int i;  
    sem_t *sem;  

    // Here may close the interrupt, confirm it is an atonic op ============

    sem = (sem_t*)kmalloc(sizeof(sem_t)); // allocate memory space
    kernel_memcpy(sem->name, name, SEM_NAME_LENGTH);
    /*
    for(i = 0; (sem->name[i] = name[i]) != '\0'; i++) // Copy name   
        ;     
    */
    sem->value = value;  
    sem->head = NULL;  
    sem->tail = NULL;  
    semaphore[sem_num]=sem;  
    sem_num++;      
    
    // Before return, open the interrupt, confirn it is an atonic op ===================
    return sem;  
}  
  
int sem_wait(sem_t *sem)  
{  
    queue *tmp=NULL;  
    if(!sem)  
    {  
        return -1;  
    }  

    // Here may close the interrupt, confirm it is an atonic op ============

     sem->value--;  
    if(sem->value<0)  
    {  
        tmp = (queue*)get_free_page();//malloc(sizeof(queue));  
        tmp->next = NULL;  
        if(sem->head==NULL)  
        {  
            sem->head = tmp;  
            sem->tail = tmp;  
        }  
        else  
        {  
            sem->tail->next = tmp;  
            sem->tail = tmp;  
        }  
        //printk("sleep %d\n",sys_getpid());  
        sleep_on(&tmp->point);                //sleep_on的参数是struct task_struct**,需要传入queue中的point,刚开始忘记写point了。。  
    }    

    // Before return, open the interrupt, confirn it is an atonic op ===================
    return 0;  
}  
  
int sem_post(sem_t *sem)  
{  
    if(!sem)  
    {  
        return -1;  
    }  
    cli();  
    sem->value++;  
    //printk("post %s %d\n",sem->name,sem->value);  
    if(sem->value<=0)  
    {  
        if(sem->head != NULL)  
        {  
            queue *p = sem->head;  
            //printk("wake %d\n",sem->head->point->pid);  
            wake_up(&sem->head->point);  
            sem->head = sem->head->next;  
            free_page((long)p);                     //同malloc原理一样，若只改malloc会报错，free_page和get_free_page配套使用  
        }  
    }  
    sti();  
    return 0;  
}  
  
int sem_unlink(const char *name)  
{  
    cli();  
    int i;  
    for(i=0; i<sem_num; i++)  
    {  
        if(semaphore[i] != NULL && strcmp(semaphore[i]->name,name)==0)  
        {  
            free_page((long)semaphore[i]);  
            break;  
        }  
    }  
    sti();  
    return i==sem_num?-1:0;  
}  