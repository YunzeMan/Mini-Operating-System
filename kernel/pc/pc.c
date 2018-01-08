#include "pc.h"

#include <driver/vga.h>
#include <intr.h>
#include <zjunix/syscall.h>
#include <zjunix/utils.h>

task_level task_queue[8]; // 
int curr_proc[8];
int curr_queue;

unsigned int time_slice[8]; // 

static void copy_context(context* src, context* dest) {
    dest->epc = src->epc;
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

void init_pc() {
    // kernel_puts("Init_pc_start\n", 0xfff, 0);
    int i, j;
    for (i = 0; i < 8; i++)
    { // 
        for(j = 0; j < 8; j++)
            task_queue[i].pcb[j].ASID = -1;
        curr_proc[i] = -1;

        time_slice[i] = 1000000 * (10 - i);
    }


    // 
    task_queue[4].pcb[0].ASID = 0;
    task_queue[4].pcb[0].counter = PROC_DEFAULT_TIMESLOTS;
    kernel_strcpy(task_queue[4].pcb[0].name, "init");
    curr_proc[4] = 0;
    curr_queue = 4;
    register_syscall(10, pc_kill_syscall);
    register_syscall(11, pc_preempt_syscall);
    register_interrupt_handler(7, pc_schedule);

    asm volatile(
        "li $v0, 100000000\n\t" 
        "mtc0 $v0, $11\n\t" // 
        "mtc0 $zero, $9\n\t"   // 
        ); 
    // kernel_puts("Init_pc_end\n", 0xfff, 0);

}

void pc_schedule(unsigned int status, unsigned int cause, context* pt_context) {
    //kernel_puts("pc_schedule start\n", 0xfff, 0);
    unsigned int compare_time;
    // Save context
    copy_context(pt_context, &(task_queue[curr_queue].pcb[curr_proc[curr_queue]].context));
    // Save current time slice
    asm volatile(
        "mfc0 %0, $11\n\t"
        "mfc0 %1, $9\n\t"
        "mtc0 $zero, $9\n\t"
        : "=r"(compare_time), "=r"(task_queue[curr_queue].pcb[curr_proc[curr_queue]].time_slice));
    
    // Decrement or increment the priority by 1
    if (task_queue[curr_queue].pcb[curr_proc[curr_queue]].time_slice < compare_time) // Be preempted, increment priority
    { // If not the highest priority
        //kernel_puts("preempted start\n", 0xfff, 0);

        if (curr_queue >= 4 && curr_queue != 7)
        {
            int k;
            for (k = 0; k < 8; k++)
            {
                curr_proc[curr_queue + 1] = (curr_proc[curr_queue + 1] + 1) & 7;
                if(task_queue[curr_queue + 1].pcb[curr_proc[curr_queue + 1]].ASID < 0)
                    break;
            }
            if(k != 8) // Then the upper priority is not full, curr_proc is updated
            {
                // Copy the PCB
                task_queue[curr_queue + 1].pcb[curr_proc[curr_queue + 1]].context.epc = task_queue[curr_queue].pcb[curr_proc[curr_queue]].context.epc;
                task_queue[curr_queue + 1].pcb[curr_proc[curr_queue + 1]].context.sp = task_queue[curr_queue].pcb[curr_proc[curr_queue]].context.sp;
                task_queue[curr_queue + 1].pcb[curr_proc[curr_queue + 1]].context.gp = task_queue[curr_queue].pcb[curr_proc[curr_queue]].context.gp;
                task_queue[curr_queue + 1].pcb[curr_proc[curr_queue + 1]].priority = task_queue[curr_queue].pcb[curr_proc[curr_queue]].priority;
                task_queue[curr_queue + 1].pcb[curr_proc[curr_queue + 1]].time_slice = task_queue[curr_queue].pcb[curr_proc[curr_queue]].time_slice; // Copy the time_slice
                kernel_strcpy(task_queue[curr_queue + 1].pcb[curr_proc[curr_queue + 1]].name, task_queue[curr_queue].pcb[curr_proc[curr_queue]].name);
                task_queue[curr_queue + 1].pcb[curr_proc[curr_queue + 1]].ASID = task_queue[curr_queue].pcb[curr_proc[curr_queue]].ASID;

                task_queue[curr_queue].pcb[curr_proc[curr_queue]].ASID = -1;

                curr_queue = curr_queue + 1;
            }
        }
    }
    else // time slice == compare time, decrement the priority
    {
        //kernel_puts("time slice runout start\n", 0xfff, 0);

        // Re-assign the time-slice for process
        task_queue[curr_queue].pcb[curr_proc[curr_queue]].time_slice = 0;
        if (curr_queue > 4)
        {
            int k;
            for (k = 0; k < 8; k++)
            {
                curr_proc[curr_queue - 1] = (curr_proc[curr_queue - 1] + 1) & 7;
                if(task_queue[curr_queue - 1].pcb[curr_proc[curr_queue - 1]].ASID < 0)
                    break;
            }
            if(k != 8) // Then the lower priority is not full, curr_proc is updated
            {
                // Copy the PCB
                task_queue[curr_queue - 1].pcb[curr_proc[curr_queue - 1]].context.epc = task_queue[curr_queue].pcb[curr_proc[curr_queue]].context.epc;
                task_queue[curr_queue - 1].pcb[curr_proc[curr_queue - 1]].context.sp = task_queue[curr_queue].pcb[curr_proc[curr_queue]].context.sp;
                task_queue[curr_queue - 1].pcb[curr_proc[curr_queue - 1]].context.gp = task_queue[curr_queue].pcb[curr_proc[curr_queue]].context.gp;
                task_queue[curr_queue - 1].pcb[curr_proc[curr_queue - 1]].priority = task_queue[curr_queue].pcb[curr_proc[curr_queue]].priority;
                task_queue[curr_queue - 1].pcb[curr_proc[curr_queue - 1]].time_slice = task_queue[curr_queue].pcb[curr_proc[curr_queue]].time_slice; // Copy the time_slice
                kernel_strcpy(task_queue[curr_queue - 1].pcb[curr_proc[curr_queue - 1]].name, task_queue[curr_queue].pcb[curr_proc[curr_queue]].name);
                task_queue[curr_queue - 1].pcb[curr_proc[curr_queue - 1]].ASID = task_queue[curr_queue].pcb[curr_proc[curr_queue]].ASID;
                
                task_queue[curr_queue].pcb[curr_proc[curr_queue]].ASID = -1;
                curr_queue = curr_queue - 1;
            }
        }
    }
    //kernel_puts("pc_schedule step1\n", 0xfff, 0);
    // Select process to load in CPU
    int i, j;
    for(i = 7; i >= 0; i--) // From high priority to low priority
    {
       //kernel_printf("i is %d\n", i);

        for(j = 0; j < 8; j++)
        {
            curr_proc[i] = (curr_proc[i] + 1) & 7;
            //kernel_printf("curr_proc[i] is %d\n", curr_proc[i]);

            if(task_queue[i].pcb[curr_proc[i]].ASID > 0){
                //kernel_printf("i is %d, ASID is %d\n",i, task_queue[i].pcb[curr_proc[i]].ASID);
                break;
            }

        }
        //kernel_printf("here\n");
        //kernel_printf("j == %d\n", j);
        if (j != 8){
            //
            break;
        }
    }
    //kernel_puts("pc_schedule step2\n", 0xfff, 0);
    // If no valid process can be loaded, error!
    if(i < 0){
        kernel_puts("Error: PCB[0] is invalid!\n", 0xfff, 0);
        while (1)
            ;
    }
    curr_queue = i;

    // Load context
    copy_context(&(task_queue[curr_queue].pcb[curr_proc[curr_queue]].context), pt_context);
    //kernel_puts("pc_schedule step3\n", 0xfff, 0);
    asm volatile( 
        "mtc0 %0, $11\n\t" // 
        "mtc0 %1, $9\n\t"   //
        : "=r"(time_slice[task_queue[curr_queue].pcb[curr_proc[curr_queue]].priority]), "=r"(task_queue[curr_queue].pcb[curr_proc[curr_queue]].time_slice)); // compare time is set according to static priority
    //kernel_puts("pc_schedule end\n", 0xfff, 0);

}

int pc_peek(int priority) {
    int i = 0;
    for (i = 0; i < 8; i++)
        if (task_queue[priority].pcb[i].ASID < 0)
            break;
    if (i == 8)
        return -1;
    return i;
}

void pc_create(int asid, void (*func)(), unsigned int init_sp, unsigned int init_gp, char* name, int priority) { // curr proc cannot change
    //kernel_puts("pc_create start\n", 0xfff, 0);
    int i;
    int curr_pos = curr_proc[priority]; // Do not change the curr_proc value
    for (i = 0; i < 8; i++)
    {
        curr_pos = (curr_pos + 1) & 7;
        if(task_queue[priority].pcb[curr_pos].ASID < 0)
            break;
    }
    if(i == 8)
    {
        kernel_puts("Create Error: Priority queue is full!\n", 0xfff, 0);
        while (1)
            ;
    }

    task_queue[priority].pcb[curr_pos].context.epc = (unsigned int)func;
    task_queue[priority].pcb[curr_pos].context.sp = init_sp;
    task_queue[priority].pcb[curr_pos].context.gp = init_gp;
    task_queue[priority].pcb[curr_pos].priority = priority; // Static priority
    task_queue[priority].pcb[curr_pos].time_slice = 0;
    kernel_strcpy(task_queue[priority].pcb[curr_pos].name, name);
    task_queue[priority].pcb[curr_pos].ASID = asid;
    
    // 
    if (priority > curr_queue)
    { //  
        //kernel_puts("Preempt start\n", 0xfff, 0);
        asm volatile(
            "li $v0, 11\n\t"
            "syscall\n\t"
            "nop\n\t");
    }
    //kernel_puts("pc_create end\n", 0xfff, 0);

}

void pc_preempt_syscall(unsigned int status, unsigned int cause, context* pt_context){ // 
    pc_schedule(status, cause, pt_context);
}


void pc_kill_syscall(unsigned int status, unsigned int cause, context* pt_context) {
    if (curr_queue != 0 || curr_proc[0] != 0) {
        task_queue[curr_queue].pcb[curr_proc[curr_queue]].ASID = -1;
        pc_schedule(status, cause, pt_context);
    }
}

int pc_kill(int proc) {
    //proc &= 7;
    if (proc == 0){
        kernel_puts("Error: Init process cannot be killed!\n", 0xfff, 0);
        return 0;
    }
    if (proc == 1){
        kernel_puts("Error: Powershell process cannot be killed!\n", 0xfff, 0);
        return 1;
    }
    int i, j;
    for(i = 7; i >= 0; i--) // From high priority to low priority
    {
        for(j = 0; j < 8; j++)
        {
            curr_proc[i] = (curr_proc[i] + 1) & 7;
            if(task_queue[i].pcb[curr_proc[i]].ASID = proc){
                task_queue[i].pcb[curr_proc[i]].ASID = -1;
                return 2;
            }    
        }
    }
    if(i < 0)
    {
        kernel_puts("Error: No process with input ID is found!\n", 0xfff, 0);
        return 3;
    }
    return -1;
}

task_struct* get_curr_pcb() {
    return &task_queue[curr_queue].pcb[curr_proc[curr_queue]];
}

int print_proc() {
    int i, j;
    kernel_puts("PID name\n", 0xfff, 0);
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++){
            if (task_queue[i].pcb[j].ASID >= 0)
                kernel_printf(" %x  %s\n", task_queue[i].pcb[j].ASID, task_queue[i].pcb[j].name);
        }
    }
    return 0;
}

// schedule, create and init are carefully checked