#ifndef _ZJUNIX_FSCACHE_H
#define _ZJUNIX_FSCACHE_H

#include <zjunix/type.h>

/* 4k byte buffer */
typedef struct buf_4k {
    u8 buf[4096];
    u32 cur;
    u32 state;
} BUF_4K;

/* 2k byte buffer */
typedef struct buf_2k {
    u8 buf[2048];
    u32 cur;
    u32 state;
} BUF_2K;

/* 1k byte buffer */
typedef struct buf_1k {
    u8 buf[1024];
    u32 cur;
    u32 state;
} BUF_1K;

/* 512 byte buffer */
typedef struct buf_512 {
    u8 buf[512];
    u32 cur;
    u32 state;
} BUF_512;

u32 fs_victim_4k(BUF_4K *buf, u32 *clock_head, u32 size);
u32 fs_write_4k(BUF_4K *f);
u32 fs_read_4k(BUF_4K *f, u32 FirstSectorOfCluster, u32 *clock_head, u32 size);
u32 fs_clr_4k(BUF_4K *buf, u32 *clock_head, u32 size, u32 cur);

u32 fs_victim_2k(BUF_2K *buf, u32 *clock_head, u32 size);
u32 fs_write_2k(BUF_2K *f);
u32 fs_read_2k(BUF_2K *f, u32 FirstSectorOfCluster, u32 *clock_head, u32 size);
u32 fs_clr_2k(BUF_2K *buf, u32 *clock_head, u32 size, u32 cur);

u32 fs_victim_1k(BUF_1K *buf, u32 *clock_head, u32 size);
u32 fs_write_1k(BUF_1K *f);
u32 fs_read_1k(BUF_1K *f, u32 FirstSectorOfCluster, u32 *clock_head, u32 size);
u32 fs_clr_1k(BUF_1K *buf, u32 *clock_head, u32 size, u32 cur);

u32 fs_victim_512(BUF_512 *buf, u32 *clock_head, u32 size);
u32 fs_write_512(BUF_512 *f);
u32 fs_read_512(BUF_512 *f, u32 FirstSectorOfCluster, u32 *clock_head, u32 size);
u32 fs_clr_512(BUF_512 *buf, u32 *clock_head, u32 size, u32 cur);

#endif //!_ZJUNIX_FSCACHE_H