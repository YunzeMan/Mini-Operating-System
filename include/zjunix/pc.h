#ifndef _ZJUNIX_PC_H
#define _ZJUNIX_PC_H

typedef struct {
    unsigned int epc;
    unsigned int at;
    unsigned int v0, v1;
    unsigned int a0, a1, a2, a3;
    unsigned int t0, t1, t2, t3, t4, t5, t6, t7;
    unsigned int s0, s1, s2, s3, s4, s5, s6, s7;
    unsigned int t8, t9;
    unsigned int hi, lo;
    unsigned int gp;
    unsigned int sp;
    unsigned int fp;
    unsigned int ra;
} context;

/* Most important part
 * 
 * Save all the information of a process, including its PID (ASID),
 * context info, name, time_slice and static priority.
 *
 * With this struct we are able to control a process.
 */
#define BASIC_TIME_SLICE 10000000 // Basic time slice of the init process

typedef struct {
    context context;
    int ASID;
    int PPID;
    unsigned int counter;
    char name[32];
    unsigned long start_time;
    unsigned int time_slice;

    unsigned int priority;
    unsigned int stack;

} task_struct;


typedef struct {
    task_struct pcb[16];
} task_level;

typedef union {
    task_struct task;
    unsigned char kernel_stack[8192];
} task_union;

#define PROC_DEFAULT_TIMESLOTS 6

void init_pc();
void pc_schedule(unsigned int status, unsigned int cause, context* pt_context);
int pc_peek(int priority);
void pc_create(int asid, void (*func)(), unsigned int init_sp, unsigned int init_gp, char* name, int priority);
void do_fork(unsigned int status, unsigned int cause, context* pt_context);
int fork();
void pc_kill_syscall(unsigned int status, unsigned int cause, context* pt_context);
void pc_preempt_syscall(unsigned int status, unsigned int cause, context* pt_context);
int pc_kill(int proc);
task_struct* get_curr_pcb();
int print_proc();
void test_fork();
void exit();


/*
 * Task state bitmask. Similar to the one in Linux
 *
 * Running tasks have a state of 0
 * Interruptible tasks have a state of 1
 * Uninterruptible tasks have a state of 2
 * Stopped tasks have a state of 4
 * Traced tasks have a state of 8
 * Dead tasks have a state of 16
 * Wakekill tasks have a state of 32
 * Waking tasks have a state of 64
 * MAX is 128
 */
#define TASK_RUNNING		0
#define TASK_INTERRUPTIBLE	1
#define TASK_UNINTERRUPTIBLE	2
#define __TASK_STOPPED		4
#define __TASK_TRACED		8

#define TASK_DEAD		16
#define TASK_WAKEKILL		32
#define TASK_WAKING		64
#define TASK_STATE_MAX		128

/*
 * Priority of a process goes from 0..MAX_PRIO-1, valid RT
 * priority is 0..MAX_RT_PRIO-1, and SCHED_NORMAL/SCHED_BATCH
 * tasks are in the range MAX_RT_PRIO..MAX_PRIO-1. Priority
 * values are inverted: lower p->prio value means higher priority.
 *
 * The MAX_USER_RT_PRIO value allows the actual maximum
 * RT priority to be separate from the value exported to
 * user-space.  This allows kernel threads to set their
 * priority to a value higher than any user task. Note:
 * MAX_RT_PRIO must not be smaller than MAX_USER_RT_PRIO.
 */
#define MAX_PRIO		    7
#define DEFAULT_PRIO		4

#endif  // !_ZJUNIX_PC_H