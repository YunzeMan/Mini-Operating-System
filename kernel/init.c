#include <arch.h>
#include <driver/ps2.h>
#include <driver/vga.h>
#include <exc.h>
#include <intr.h>
#include <page.h>
#include <zjunix/bootmm.h>
#include <zjunix/buddy.h>
#include <zjunix/fs/fat.h>
#include <zjunix/log.h>
#include <zjunix/pc.h>
#include <zjunix/pid.h>
#include <zjunix/slab.h>
#include <zjunix/syscall.h>
#include <zjunix/time.h>
#include <zjunix/fs/vfs.h>
#include <zjunix/fs/tree.h>
#include "../usr/ps.h"

void machine_info() {
    int row;
    int col;
    kernel_printf("\n  %s\n", "Mini_Operating_System V1.0");
    row = cursor_row;
    col = cursor_col;
    cursor_row = 29;
    kernel_printf("  %s", "Created by OS Group, MAN Yunze, BIAN Song, CHEN jiaao.");
    cursor_row = row;
    cursor_col = col;
    kernel_set_cursor();
}

#pragma GCC push_options
#pragma GCC optimize("O0")
void create_startup_process() {
    kernel_puts("  create_startup_process_start\n", 0xfff, 0);
    unsigned int init_gp;
    asm volatile("la %0, _gp\n\t" : "=r"(init_gp));
    int asid_1, asid_2;
    asid_1 = alloc_pidmap();
    kernel_printf("  asid1 is %d\n", asid_1);
    pc_create(asid_1, ps, (unsigned int)kmalloc(8192) + 8192, init_gp, "Shell", DEFAULT_PRIO + 2);
    log(LOG_OK, "Shell init");
    asid_2 = alloc_pidmap();
    kernel_printf("  asid2 is %d\n", asid_2);
    pc_create(asid_2, system_time_proc, (unsigned int)kmalloc(8192) + 8192, init_gp, "time", DEFAULT_PRIO + 1);
    log(LOG_OK, "Timer init");
}
#pragma GCC pop_options

void init_kernel() {
    kernel_clear_screen(31);
    // Exception
    init_exception();
    // Page table
    init_pgtable();
    // Drivers
    init_vga();
    init_ps2();
    // Memory management
    log(LOG_START, "Memory Modules.");
    init_bootmm();
    log(LOG_OK, "Bootmem.");
    init_buddy();
    log(LOG_OK, "Buddy.");
    init_slab();
    log(LOG_OK, "Slab.");
    log(LOG_END, "Memory Modules.");
    // File system
    log(LOG_START, "File System.");
    init_fs();
    log(LOG_END, "File System.");
    // Virtual File System
    log(LOG_START, "Virtual File System.");
    // new add vfs
    initial_vfs();
    log(LOG_OK, "Virtual File System.");
    log(LOG_END, "Virtual File System.");
    // File Tree
    log(LOG_START, "File Tree.");
    // new add filetree
    init_filetree();
    log(LOG_OK, "File Tree.");
    log(LOG_END, "File Tree.");
    // System call
    log(LOG_START, "System Calls.");
    init_syscall();
    log(LOG_END, "System Calls.");
    // Process control
    log(LOG_START, "Process Control Module.");
    init_pid();
    init_pc();
    create_startup_process();
    log(LOG_END, "Process Control Module.");
    // Interrupts
    log(LOG_START, "Enable Interrupts.");
    init_interrupts();
    log(LOG_END, "Enable Interrupts.");
    // Init finished
    machine_info();
    *GPIO_SEG = 0x11223344;
    // Enter shell
    while (1)
        ;
}
