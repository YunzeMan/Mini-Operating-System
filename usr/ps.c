#include "ps.h"
#include <driver/ps2.h>
#include <driver/sd.h>
#include <driver/vga.h>
#include <zjunix/bootmm.h>
#include <zjunix/buddy.h>
#include <zjunix/fs/fat.h>
#include <zjunix/pid.h>
#include <zjunix/slab.h>
#include <zjunix/time.h>
#include <zjunix/utils.h>
#include <zjunix/fs/vfs.h>
#include <zjunix/fs/tree.h>
#include "cd.h"
#include "../usr/ls.h"
#include "exec.h"
#include "myvi.h"

char ps_buffer[64];
int ps_buffer_index;
char pwd[256];

void test_syscall4() {
    asm volatile(
        "li $a0, 0x00ff\n\t"
        "li $v0, 4\n\t"
        "syscall\n\t"
        "nop\n\t");
}

void test_proc() {
    unsigned int timestamp;
    unsigned int currTime;
    unsigned int data;
    asm volatile("mfc0 %0, $9, 6\n\t" : "=r"(timestamp));
    data = timestamp & 0xff;
    while (1) {
        asm volatile("mfc0 %0, $9, 6\n\t" : "=r"(currTime));
        if (currTime - timestamp > 100000000) {
            timestamp += 100000000;
            *((unsigned int *)0xbfc09018) = data;
        }
    }
}

int proc_demo_create() {
    int asid = pc_peek(4);
    if (asid < 0) {
        kernel_puts("  Failed to allocate pid.\n", 0xfff, 0);
        return 1;
    }
    unsigned int init_gp;
    asm volatile("la %0, _gp\n\t" : "=r"(init_gp));
    pc_create(asid, test_proc, (unsigned int)kmalloc(8192), init_gp, "test", 4);
    return 0;
}

void ps() {
    kernel_printf("  Press any key to enter shell.\n");
    kernel_getchar();
    char c;
    int i;
    for(i = 0; i < 256; i++)
    {
        pwd[i] = 0;
    }
    pwd[0] = '/';
    ps_buffer_index = 0;
    ps_buffer[0] = 0;
    kernel_clear_screen(31);
    kernel_puts("  PowerShell\n", 0xfff, 0);
    kernel_puts("  PS>", 0xfff, 0);
    while (1) {
        c = kernel_getchar();
        if (c == '\n') {
            ps_buffer[ps_buffer_index] = 0;
            if (kernel_strcmp(ps_buffer, "exit") == 0) {
                ps_buffer_index = 0;
                ps_buffer[0] = 0;
                kernel_printf("  \nPowerShell exit.\n");
            } else
                parse_cmd();
            ps_buffer_index = 0;
            kernel_puts("  PS>", 0xfff, 0);
        } else if (c == 0x08) {
            if (ps_buffer_index) {
                ps_buffer_index--;
                kernel_putchar_at(' ', 0xfff, 0, cursor_row, cursor_col - 1);
                cursor_col--;
                kernel_set_cursor();
            }
        } else {
            if (ps_buffer_index < 63) {
                ps_buffer[ps_buffer_index++] = c;
                kernel_putchar(c, 0xfff, 0);
            }
        }
    }
}

void parse_cmd() {
    unsigned int result = 0;
    char dir[32];
    char c;
    kernel_putchar('\n', 0, 0);
    char sd_buffer[2048];
    int i = 0;
    int j;
    int len; /* The length of string */
    char *param; /* The parameter of instruction */
    char *src; /* The source of cp and mv instruction */
    char *dest; /* The destination of cp and mv instruction */
    char *parent;
    struct filetree * fd;
    /* participle ps_buffer to get param and cmd */
    for (i = 0; i < 63; i++) {
        if (ps_buffer[i] == ' ') {
            ps_buffer[i] = 0;
            break;
        }
    }
    if (i == 63)
        param = ps_buffer;
    else
        param = ps_buffer + i + 1;
    if (ps_buffer[0] == 0) {
        return;
    } else if (kernel_strcmp(ps_buffer, "clear") == 0) {
        kernel_clear_screen(31);
    } else if (kernel_strcmp(ps_buffer, "echo") == 0) {
        kernel_printf("  %s\n", param);
    } else if (kernel_strcmp(ps_buffer, "gettime") == 0) {
        char buf[10];
        get_time(buf, sizeof(buf));
        kernel_printf("  %s\n", buf);
    } else if (kernel_strcmp(ps_buffer, "syscall4") == 0) {
        test_syscall4();
    } else if (kernel_strcmp(ps_buffer, "sdwi") == 0) {
        for (i = 0; i < 512; i++)
            sd_buffer[i] = i;
        sd_write_block(sd_buffer, 7, 1);
        kernel_puts("  sdwi\n", 0xfff, 0);
    } else if (kernel_strcmp(ps_buffer, "sdr") == 0) {
        sd_read_block(sd_buffer, 7, 1);
        for (i = 0; i < 512; i++) {
            kernel_printf("  %d ", sd_buffer[i]);
        }
        kernel_putchar('\n', 0xfff, 0);
    } else if (kernel_strcmp(ps_buffer, "sdwz") == 0) {
        for (i = 0; i < 512; i++) {
            sd_buffer[i] = 0;
        }
        sd_write_block(sd_buffer, 7, 1);
        kernel_puts("  sdwz\n", 0xfff, 0);
    } else if (kernel_strcmp(ps_buffer, "mminfo") == 0) {
        bootmap_info("bootmm");
        buddy_info();
    } else if (kernel_strcmp(ps_buffer, "mmtest") == 0) {
        kernel_printf("  kmalloc : %x, size = 1KB\n", kmalloc(1024));
    } else if (kernel_strcmp(ps_buffer, "ps") == 0) {
        result = print_proc();
        kernel_printf("  ps return with %d\n", result);
    } else if (kernel_strcmp(ps_buffer, "forktest") == 0) {
        unsigned int init_gp;
        asm volatile("la %0, _gp\n\t" : "=r"(init_gp));
        int asid = alloc_pidmap();
        pc_create(asid, test_fork, (unsigned int)kmalloc(8192) + 8192, init_gp, "forktest", DEFAULT_PRIO + 2);
    } else if (kernel_strcmp(ps_buffer, "pidtest") == 0) {
        unsigned int init_gp;
        asm volatile("la %0, _gp\n\t" : "=r"(init_gp));
        int asid = alloc_pidmap();
        pc_create(asid, test_pidmap, (unsigned int)kmalloc(8192) + 8192, init_gp, "pidtest", DEFAULT_PRIO + 2);
    } else if (kernel_strcmp(ps_buffer, "killmode") == 0) {
        alt_kill_mode();
        kernel_printf("  Kill mode has changed!\n");
    } else if (kernel_strcmp(ps_buffer, "mmtest1") == 0) {
        void * addr1 = kmalloc(512);
        kernel_printf("  kmalloc : %x, size = 0.5KB\n", addr1);
        void * addr2 = kmalloc(512);
        kernel_printf("  kmalloc : %x, size = 0.5KB\n", addr2);
        kfree(addr1);
    
    } else if (kernel_strcmp(ps_buffer, "mmtest2") == 0) {
        void * addr1 = kmalloc(512);
        kernel_printf("  kmalloc : %x, size = 0.5KB\n", addr1);
        void * addr2 = kmalloc(512);
        kernel_printf("  kmalloc : %x, size = 0.5KB\n", addr2);
        void * addr3 = kmalloc(512);
        kernel_printf("  kmalloc : %x, size = 0.5KB\n", addr3);
    } else if (kernel_strcmp(ps_buffer, "mmtest3") == 0) {
        void * addr1 = kmalloc(1024);
        kernel_printf("  kmalloc : %x, size = 1KB\n", addr1);
        void * addr2 = kmalloc(1024);
        kernel_printf("  kmalloc : %x, size = 1KB\n", addr2);
        void * addr3 = kmalloc(1024);
        kernel_printf("  kmalloc : %x, size = 1KB\n", addr3);

    } 
    else if (kernel_strcmp(ps_buffer, "mmtest4") == 0) {
        void * addr1 = kmalloc(8192);
        kernel_printf("  kmalloc : %x, size = 8KB\n", addr1);
        void * addr2 = kmalloc(8192);
        kernel_printf("  kmalloc : %x, size = 8KB\n", addr2);
        kfree(addr1);
    }else if (kernel_strcmp(ps_buffer, "kill") == 0) {
        int pid = param[0] - '0';
        kernel_printf("  Killing process %d\n", pid);
        result = pc_kill(pid);
        kernel_printf("  kill return with %d\n", result);
    } else if (kernel_strcmp(ps_buffer, "time") == 0) {
        unsigned int init_gp;
        asm volatile("la %0, _gp\n\t" : "=r"(init_gp));
        int asid = alloc_pidmap();
        pc_create(asid, system_time_proc, (unsigned int)kmalloc(8192)+8192, init_gp, "time", DEFAULT_PRIO);
    } else if (kernel_strcmp(ps_buffer, "time7") == 0) {
        unsigned int init_gp;
        asm volatile("la %0, _gp\n\t" : "=r"(init_gp));
        int asid = alloc_pidmap();
        pc_create(asid, system_time_proc, (unsigned int)kmalloc(8192) + 8192, init_gp, "time7", MAX_PRIO); 
    } else if (kernel_strcmp(ps_buffer, "proc") == 0) {
        result = proc_demo_create();
        kernel_printf("  proc return with %d\n", result);
    } else if (kernel_strcmp(ps_buffer, "cat") == 0) {
        result = vfsfile->fat32_file->cat(param);
        //result = fs_cat(param);
        kernel_printf("  cat return with %d\n", result);
    } else if (kernel_strcmp(ps_buffer, "ls") == 0) {
        result = ls(param);
        kernel_printf("  ls return with %d\n", result);
    } else if (kernel_strcmp(ps_buffer, "vi") == 0) {
        result = myvi(param);
        kernel_printf("  vi return with %d\n", result);
    } else if (kernel_strcmp(ps_buffer, "exec") == 0) {
        result = exec(param);
        kernel_printf("  exec return with %d\n", result);
    } else if (kernel_strcmp(ps_buffer, "rm") == 0) {
        result = vfsfile->fat32_file->rm(param);
        //result = fs_rm(param);
        //deleteNode(param);
        kernel_printf("  rm return with %d\n", result);
    } else if (kernel_strcmp(ps_buffer, "mkdir") == 0) {
        result = vfsfile->fat32_file->mkdir(param);
        // add in tree
        /*struct filetree * p;
        len = strlen(param);
        init_treenode(p,param);
        for(i = len - 1; i >=0; i--)
        {
            if(param[i] == '/')
            {
                param[i] = 0;
                break;
            }
        }
        parent = param;
        fd = findNode(parent);
        becomeChild(fd,p);*/
        //result = fs_mkdir(param);
        kernel_printf("  mkdir return with %d\n", result);
    } else if (kernel_strcmp(ps_buffer, "rmdir") == 0) {
        /* add rmdir instruction */
        result = vfsfile->fat32_file->rmdir(param);
        //deleteNode(param);
        //result = fs_rmdir(param);
        kernel_printf("  rmdir return with %d\n", result);
    } else if (kernel_strcmp(ps_buffer, "touch") == 0) {
        result = vfsfile->fat32_file->create(param);
        // add in tree
        /*struct filetree * p;
        len = strlen(param);
        init_treenode(p,param);
        for(i = len - 1; i >=0; i--)
        {
            if(param[i] == '/')
            {
                param[i] = 0;
                break;
            }
        }
        parent = param;
        fd = findNode(parent);
        becomeChild(fd,p);*/
        //result = fs_create(param);
        kernel_printf("  touch return with %d\n", result);
    } else if (kernel_strcmp(ps_buffer, "ls_l") == 0) {
        result = ls_l(param);
        kernel_printf("  ls_l return with %d\n", result);
    } else if (kernel_strcmp(ps_buffer, "ls_help") == 0) {
        result = ls_help();
        kernel_printf("  ls_a return with %d\n", result);
    } else if (kernel_strcmp(ps_buffer, "pwd") == 0) {
        kernel_printf("  %s\n",pwd);
    } else if (kernel_strcmp(ps_buffer, "tree") == 0) {
        print_tree(root);
    } else if (kernel_strcmp(ps_buffer, "cd") == 0) {
        do_cd(param, pwd);
    } else if (kernel_strcmp(ps_buffer, "mv") == 0) {
        /* add mv instruction */
        for (i = 0; i < 63; i++)
        {
            if(param[i] == ' ')
            {
                param[i] = 0;
                break;
            }
        }
        if (i == 63)
        {
            src = param;
        }
        else
        {
            src = param;
            dest = param + i + 1;
        }
        //kernel_printf("%s\n", src);
        //kernel_printf("%s\n",dest);
        result = fs_mv(src, dest);
        kernel_printf("  mv return with %d\n", result);
    } else if (kernel_strcmp(ps_buffer, "cp") == 0) {
        /* add cp instruction */
        for(i = 0; i < 63; i++)
        {
            if(param[i] == ' ')
            {
                param[i] = 0;
                break;
            }
        }
        if (i == 63)
        {
            src = param;
        }
        else
        {
            src = param;
            dest = param + i + 1;
        }
        result = fs_cp(src, dest);
        kernel_printf("%s\n", src);
        kernel_printf("%s\n",dest);
        kernel_printf("  cp return with %d\n", result);
    } else {
        kernel_printf("  ");
        kernel_puts(ps_buffer, 0xfff, 0);
        kernel_puts("  : command not found\n", 0xfff, 0);
    }
}
