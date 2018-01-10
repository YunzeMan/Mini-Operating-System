#include "pc.h"

#include <driver/vga.h>
#include <intr.h>
#include <zjunix/syscall.h>
#include <zjunix/utils.h>
#include <zjunix/slab.h>
#include <zjunix/buddy.h>
#include <zjunix/pid.h>

// The 8-level task queue, higher number stands for higher priority
task_level task_queue[8];

int curr_proc[8];// 8 current positions corresponding to 8 queues
int curr_queue; // The queue in which current process runs

// Constant time slice, set according to static priority
unsigned int time_slice[8];
// Bits Map is used for saving the 
unsigned int bits_map[8];

// 
//char pc_name[108][16];

/* Copy the context of the process
 * 
 *Function copy_context basically copies 32 registers from source context to target context
 *@param src source context, can be save or load
 *@param dest destination context, can be save or load
 *@No return value
 */
static void copy_context(context* src, context* dest) {
    dest->epc = src->epc;   // Current PC address
    dest->at = src->at;
    dest->v0 = src->v0;
    dest->v1 = src->v1;
    dest->a0 = src->a0;
    dest->a1 = src->a1;
    dest->a2 = src->a2;
    dest->a3 = src->a3;
    dest->t0 = src->t0;
    dest->t1 = src->t1;
    dest->t2 = src->t2;
    dest->t3 = src->t3;
    dest->t4 = src->t4;
    dest->t5 = src->t5;
    dest->t6 = src->t6;
    dest->t7 = src->t7;
    dest->s0 = src->s0;
    dest->s1 = src->s1;
    dest->s2 = src->s2;
    dest->s3 = src->s3;
    dest->s4 = src->s4;
    dest->s5 = src->s5;
    dest->s6 = src->s6;
    dest->s7 = src->s7;
    dest->t8 = src->t8;
    dest->t9 = src->t9;
    dest->hi = src->hi;
    dest->lo = src->lo;
    dest->gp = src->gp;
    dest->sp = src->sp;
    dest->fp = src->fp;
    dest->ra = src->ra;
}

/* Initiate PC part 
 * 
 *Function init_pc initiate the process functions of the OS. It creates the first process "init", 
 *and registers two system calls as well as one interrupt。 And finally it set $9 and $11 register
 *@No parameters
 *@No return value
 */
void init_pc() {
    int i, j;
    for (i = 0; i < 8; i++)
    { // Initiate the ASID to -1, indicating the PCB empty
        for(j = 0; j < 16; j++)
            task_queue[i].pcb[j].ASID = -1;
        curr_proc[i] = -1;
        // Set the the static time slice
        time_slice[i] = 1000000 * (10 - i);
        bits_map[i] = 0;
    }
    
    // Set the Init process's ASID to 0, a special value in order not to schedule it any more
    task_queue[4].pcb[0].ASID = alloc_pidmap();
    task_queue[4].pcb[0].counter = PROC_DEFAULT_TIMESLOTS;
    task_queue[4].pcb[0].priority = 4;

    kernel_strcpy(task_queue[4].pcb[0].name, "init");   // Name is init
    //kernel_strcpy(pc_name[0], "init");   // Name is init

    curr_proc[4] = 0;
    curr_queue = 4;
    // Register the neccessary system call and interrupt
    register_syscall(10, pc_kill_syscall);
    register_syscall(11, pc_preempt_syscall);
    register_syscall(12, do_fork);
    register_interrupt_handler(7, pc_schedule);

    // Set counter and compare register
    asm volatile(
        "li $v0, 10000000\n\t"  // Put 10000000 into Compare register
        "mtc0 $v0, $11\n\t" // 
        "mtc0 $zero, $9\n\t"   // Set counter to 0
        ); 
}

/* Do the process scheduling 
 * 
 *Function pc_schedule schedules the processes, it choose the process from a multi-level queue, 
 *processes with higher priority is scheduled first. The priority is dynamic.
 *@param status input of system calls and interrupts, the state of CPU
 *@param cause input of the system calls and interrupts, the cause of the interruption
 *@param pt_context input of the system calls and interrupts, the context of the current PC
 *@No return value
 */
void pc_schedule(unsigned int status, unsigned int cause, context* pt_context) {
    // Save context
    copy_context(pt_context, &(task_queue[curr_queue].pcb[curr_proc[curr_queue]].context));
    // Save current time slice

    unsigned int compare;
    asm volatile(
        "mfc0 %0, $9\n\t"
        "mfc0 %1, $11\n\t"
        "mtc0 $zero, $9\n\t"
        : "=r"(task_queue[curr_queue].pcb[curr_proc[curr_queue]].time_slice), "=r"(compare));    

    // Decrement or increment the priority by 1
    if (task_queue[curr_queue].pcb[curr_proc[curr_queue]].time_slice < compare) // Be preempted, increment priority
    { 
        // If not the highest priority, and higher than the basic priority
        if (curr_queue >= 4 && curr_queue != 7)
        {
            int k;
            for (k = 0; k < 16; k++)
            {
                curr_proc[curr_queue + 1] = (curr_proc[curr_queue + 1] + 1) & 15;
                if(task_queue[curr_queue + 1].pcb[curr_proc[curr_queue + 1]].ASID < 0)
                    break;
            }
            if(k != 16) // Then the upper priority is not full, curr_proc is updated
            {
                // Copy the PCB
                
                copy_context(pt_context, &(task_queue[curr_queue + 1].pcb[curr_proc[curr_queue + 1]].context));                
                task_queue[curr_queue + 1].pcb[curr_proc[curr_queue + 1]].priority = task_queue[curr_queue].pcb[curr_proc[curr_queue]].priority;
                task_queue[curr_queue + 1].pcb[curr_proc[curr_queue + 1]].time_slice = task_queue[curr_queue].pcb[curr_proc[curr_queue]].time_slice; // Copy the time_slice
                kernel_strcpy(task_queue[curr_queue + 1].pcb[curr_proc[curr_queue + 1]].name, task_queue[curr_queue].pcb[curr_proc[curr_queue]].name);
                task_queue[curr_queue + 1].pcb[curr_proc[curr_queue + 1]].ASID = task_queue[curr_queue].pcb[curr_proc[curr_queue]].ASID;
                task_queue[curr_queue + 1].pcb[curr_proc[curr_queue + 1]].stack = task_queue[curr_queue].pcb[curr_proc[curr_queue]].stack;


                task_queue[curr_queue].pcb[curr_proc[curr_queue]].ASID = -1;

                curr_queue = curr_queue + 1;
            }
        }
    }else // time slice == compare time, decrement the priority
    {
        // Re-assign the time-slice for process
        task_queue[curr_queue].pcb[curr_proc[curr_queue]].time_slice = 0;
        if (curr_queue > 4)
        {
            int k;
            for (k = 0; k < 16; k++)
            {
                curr_proc[curr_queue - 1] = (curr_proc[curr_queue - 1] + 1) & 15;
                if(task_queue[curr_queue - 1].pcb[curr_proc[curr_queue - 1]].ASID < 0)
                    break;
            }
            if(k != 16) // Then the lower priority is not full, curr_proc is updated
            {
                // Copy the PCB
                copy_context(pt_context, &(task_queue[curr_queue - 1].pcb[curr_proc[curr_queue - 1]].context));
                task_queue[curr_queue - 1].pcb[curr_proc[curr_queue - 1]].priority = task_queue[curr_queue].pcb[curr_proc[curr_queue]].priority;
                task_queue[curr_queue - 1].pcb[curr_proc[curr_queue - 1]].time_slice = task_queue[curr_queue].pcb[curr_proc[curr_queue]].time_slice; // Copy the time_slice
                kernel_strcpy(task_queue[curr_queue - 1].pcb[curr_proc[curr_queue - 1]].name, task_queue[curr_queue].pcb[curr_proc[curr_queue]].name);
                task_queue[curr_queue - 1].pcb[curr_proc[curr_queue - 1]].ASID = task_queue[curr_queue].pcb[curr_proc[curr_queue]].ASID;
                task_queue[curr_queue - 1].pcb[curr_proc[curr_queue - 1]].stack = task_queue[curr_queue].pcb[curr_proc[curr_queue]].stack;

                task_queue[curr_queue].pcb[curr_proc[curr_queue]].ASID = -1;
                curr_queue = curr_queue - 1;
            }
        }
    }
    // Select process to load in CPU
    int i, j;
    for(i = 7; i >= 0; i--) // From high priority to low priority
    {
        for(j = 0; j < 16; j++)
        { 
            curr_proc[i] = (curr_proc[i] + 1) & 15;
            if(task_queue[i].pcb[curr_proc[i]].ASID > 0) // Find a process to execute, break
                break;
        }
        if (j != 16) // Find a PCB before j reaches 8
            break;
    }
    // If does not find a pcb to exec, print error
    if(i < 0){
        kernel_puts("  Error: PCB[0] is invalid!\n", 0xfff, 0);
        while (1)
            ;
    }
    curr_queue = i;

    // Load context
    copy_context(&(task_queue[curr_queue].pcb[curr_proc[curr_queue]].context), pt_context);
    
      asm volatile( 
        "mtc0 $zero, $9\n\t"
        "mtc0 %0, $11\n\t" //
        "mtc0 %1, $9\n\t" //
        :: "r"(time_slice[task_queue[curr_queue].pcb[curr_proc[curr_queue]].priority]), "r"(task_queue[curr_queue].pcb[curr_proc[curr_queue]].time_slice)); // compare time is set according to static priority
     
     /*// The annotated asm codes are standard priority process way, due to hardware problem, it cannot be implemented
     *
     * asm volatile( 
     *   "mtc0 %0, $9\n\t" //
     *   : "=r"(task_queue[curr_queue].pcb[curr_proc[curr_queue]].time_slice)); // compare time is set according to static priority
     * 
     * // A easy version
     * 
     * asm volatile("mtc0 $zero, $9\n\t");
     */
}


/* Fetch the peek process  
 * 
 *Function pc_peek fetches the peek process of task queue of a certain priority
 *@param priority the level of task queue
 *@return int Position of the peek of level[priority] queue.
 */
int pc_peek(int priority) {
    int i = 0;
    for (i = 0; i < 16; i++)
        if (task_queue[priority].pcb[i].ASID < 0)
            break;
    if (i == 16)
        return -1;
    return i;
}

/* Directly create a process  
 * 
 *Function pc_create directly creates a process, the parameters of the process is specified as inputs
 *@param asid ASID of the new process
 *@param Start position of process, epc
 *@param init_sp Start position of the stack, sp
 *@param init_gp Start position of the heap, gp
 *@param name Name of the new process
 *@param priority Static priority of the new process
 *@No return value
 */
void pc_create(int asid, void (*func)(), unsigned int init_sp, unsigned int init_gp, char* name, int priority) { // curr proc cannot change
    //kernel_puts("  pc_create start\n", 0xfff, 0);
    int i;
    int curr_pos = curr_proc[priority]; // Do not change the curr_proc value
    for (i = 0; i < 16; i++)
    {
        curr_pos = (curr_pos + 1) & 15;
        if(task_queue[priority].pcb[curr_pos].ASID < 0)
            break;
    }
    if(i == 16)
    {
        kernel_puts("  Create Error: Priority queue is full!\n", 0xfff, 0);
        while (1)
            ;
    }

    task_queue[priority].pcb[curr_pos].context.epc = (unsigned int)func;
    task_queue[priority].pcb[curr_pos].context.sp = init_sp;
    task_queue[priority].pcb[curr_pos].stack = init_sp - 8192;
    task_queue[priority].pcb[curr_pos].context.gp = init_gp;
    task_queue[priority].pcb[curr_pos].priority = priority; // Static priority
    task_queue[priority].pcb[curr_pos].time_slice = 0;
    kernel_strcpy(task_queue[priority].pcb[curr_pos].name, name);
    //kernel_strcpy(pc_name[asid], name);   // Name is init
    
    task_queue[priority].pcb[curr_pos].ASID = asid;
    
    // 
    if (task_queue[curr_queue].pcb[curr_proc[curr_queue]].ASID > 0 && priority > curr_queue)
    { //  
        asm volatile(
            "li $v0, 11\n\t"
            "syscall\n\t"
            "nop\n\t");
    }
    //kernel_puts("  pc_create end\n", 0xfff, 0);

}

/* Do fork =======================================================
 * 
 *Function pc_preempt_syscall is registered as a system call, which calls pc_schedule 
 *@param status input of system calls and interrupts, the state of CPU
 *@param cause input of the system calls and interrupts, the cause of the interruption
 *@param pt_context input of the system calls and interrupts, the context of the current PC
 *@No return value
 */
void do_fork(unsigned int status, unsigned int cause, context* pt_context) {
    int i;
    unsigned int priority = task_queue[curr_queue].pcb[curr_proc[curr_queue]].priority;
    //kernel_printf("  a0 is %d\n",pt_context->a0);
    int curr_pos = curr_proc[priority]; // Do not change the curr_proc value
    for (i = 0; i < 16; i++)
    {
        curr_pos = (curr_pos + 1) & 15;        
        //kernel_printf("  Priority is %d\n",priority);
        //kernel_printf("  curr_pos is %d\n",curr_pos);
        //kernel_printf("  ASID is %d\n", task_queue[priority].pcb[curr_pos].ASID);
        if(task_queue[priority].pcb[curr_pos].ASID < 0)
            break;
    }
    if(i == 16)
    {
        kernel_puts("  Create Error: Priority queue is full!\n", 0xfff, 0);
        while (1)
            ;
    }
    //copy_context(pt_context, &(task_queue[curr_queue].pcb[curr_proc[curr_queue]].context));  // Save old state

    copy_context(pt_context, &(task_queue[priority].pcb[curr_pos].context)); // Copy to new 
    task_queue[priority].pcb[curr_pos].priority = priority; // Static priority
    task_queue[priority].pcb[curr_pos].time_slice = 0;
    kernel_strcpy(task_queue[priority].pcb[curr_pos].name, task_queue[curr_queue].pcb[curr_proc[curr_queue]].name);
    task_queue[priority].pcb[curr_pos].ASID = alloc_pidmap();
    pt_context->v0 = task_queue[priority].pcb[curr_pos].ASID;
    task_queue[priority].pcb[curr_pos].context.v0 = 0;

    unsigned int offset = pt_context->sp - task_queue[curr_queue].pcb[curr_proc[curr_queue]].stack;
    task_queue[priority].pcb[curr_pos].stack = (unsigned int)kmalloc(8192);
    kernel_memcpy((void *)task_queue[priority].pcb[curr_pos].stack, (void *)task_queue[curr_queue].pcb[curr_proc[curr_queue]].stack, 8192);
    task_queue[priority].pcb[curr_pos].context.sp = task_queue[priority].pcb[curr_pos].stack + offset;
  
    kernel_printf("  New Stack is %x\n", task_queue[priority].pcb[curr_pos].stack);
    kernel_printf("  Old Stack is %x\n", task_queue[curr_queue].pcb[curr_proc[curr_queue]].stack);
    kernel_printf("  New SP is %x\n", task_queue[priority].pcb[curr_pos].context.sp);
    kernel_printf("  Old SP is %x\n", pt_context->sp);

    kernel_printf("  ASID is %d\n", task_queue[priority].pcb[curr_pos].ASID);
    
    copy_context(&(task_queue[priority].pcb[curr_pos].context), pt_context);

    //print_proc();
    /*
    asm volatile(
        "li $v0, 11\n\t"
        "syscall\n\t"   
        "nop\n\t");
    */
    
    //kernel_printf("  23333 curr_pos is %d\n",curr_pos);
}

/* Do fork, copy the current process to a new one 
 * 
 *Function fork is registered as a system call, which calls pc_schedule 
 *@param status input of system calls and interrupts, the state of CPU
 *@param cause input of the system calls and interrupts, the cause of the interruption
 *@param pt_context input of the system calls and interrupts, the context of the current PC
 *@No return value
 */
int fork() {
    unsigned int pid;
    //kernel_printf("  Priority is %d\n", task_queue[curr_queue].pcb[curr_proc[curr_queue]].priority);

    asm volatile(
        "li $v0, 12\n\t"
        "syscall\n\t"
        "nop\n\t");

    asm volatile(
        "move %0, $v0\n\t"
        "nop\n\t"
        : "=r" (pid));
    
    kernel_printf("  ASID is %d\n", pid);

    return pid;
}

/* Do fork =========================================
 * 
 *Function pc_preempt_syscall is registered as a system call, which calls pc_schedule 
 *@param status input of system calls and interrupts, the state of CPU
 *@param cause input of the system calls and interrupts, the cause of the interruption
 *@param pt_context input of the system calls and interrupts, the context of the current PC
 *@No return value
 */
int test_fork() {
    unsigned int pid = fork();
    kernel_printf("  ASID is %d\n", pid);

    if (pid < 0) {
        kernel_printf("  fork failed!\n");
    }
    else if (pid == 0) {
        // In child process
        kernel_printf("  This is child process! Hello daddy!\n");
        kernel_printf("  pid is %d\n", pid);

    } 
    else {
        kernel_printf("  This is parent process! Good Son!\n");
    }
    kernel_printf("  ==========================\n");

    return pid;
}

/* Do schedule due to preemption
 * 
 *Function pc_preempt_syscall is registered as a system call, which calls pc_schedule 
 *@param status input of system calls and interrupts, the state of CPU
 *@param cause input of the system calls and interrupts, the cause of the interruption
 *@param pt_context input of the system calls and interrupts, the context of the current PC
 *@No return value
 */
void pc_preempt_syscall(unsigned int status, unsigned int cause, context* pt_context){
    pc_schedule(status, cause, pt_context);
}


void exit(){
    asm volatile(
        "li $v0, 10\n\t"
        "syscall\n\t"
        "nop\n\t");
}


/* Kill the running process
 * 
 *Function pc_kill_syscall is registered as a system call, which kills the running process by changing
 *its ASID to -1 and calling schedule function
 *@param status input of system calls and interrupts, the state of CPU
 *@param cause input of the system calls and interrupts, the cause of the interruption
 *@param pt_context input of the system calls and interrupts, the context of the current PC
 *@No return value
 */
void pc_kill_syscall(unsigned int status, unsigned int cause, context* pt_context) {
    free_pidmap(task_queue[curr_queue].pcb[curr_proc[curr_queue]].ASID);
    task_queue[curr_queue].pcb[curr_proc[curr_queue]].ASID = -1;
    pc_schedule(status, cause, pt_context);
}

/* Kill a process with certain ID
 * 
 *Function pc_kill kills a process with certain PID, 
 *its difference with pc_kill_syscall is it is not a system call.
 *@param proc ASID of the process to be killed
 *@return int The status of the function, whether the process is killed properly
 */
int pc_kill(int proc) {
    // If is init, not allowed to kill
    if (proc == 0){
        kernel_puts("  Error: Init process cannot be killed!\n", 0xfff, 0);
        return 0;
    }
    // If is powershell, not allowed to kill    
    if (proc == 1){
        kernel_puts("  Error: Powershell process cannot be killed!\n", 0xfff, 0);
        return 1;
    }

    int i, j;
    // traverse from high priority to low priority 
    for(i = 7; i >= 0; i--) 
    {
        for(j = 0; j < 16; j++)
        {
            curr_proc[i] = (curr_proc[i] + 1) & 15;
            if(task_queue[i].pcb[curr_proc[i]].ASID = proc){
                free_pidmap(proc);
                task_queue[i].pcb[curr_proc[i]].ASID = -1;
                return 2; // return 2 if correctly killed
            }    
        }
    }
    if(i < 0)
    { // Print error info, return 3
        kernel_puts("  Error: No process with input ID is found!\n", 0xfff, 0);
        return 3;
    }
    return -1; // If return -1, something bad happens
}

/* Fetch the PCB of the running process  
 * 
 *Function get_curr_pcb fetches the pcb of the running process
 *@No param
 *@return task_struct* The fetched PCB of current process
 */
task_struct* get_curr_pcb() {
    return &task_queue[curr_queue].pcb[curr_proc[curr_queue]];
}

/* Print all processes in the OS 
 * 
 *Function print_proc prints all processes in the OS
 *@No param
 *@return int The status of the function, whether the process is printed properly
 */
int print_proc() {
    int i, j;
    kernel_puts("  PID\t\tname\t\tstatP\n", 0xfff, 0);
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 16; j++){
            if (task_queue[i].pcb[j].ASID >= 0)
                kernel_printf("   %x\t\t%s\t\t%d\n", task_queue[i].pcb[j].ASID, task_queue[i].pcb[j].name, task_queue[i].pcb[j].priority);
        }
    }
    return 0;
}

// schedule, create and init are carefully checked