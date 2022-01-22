#pragma once
#include "../global.h"
#include "../cpu_info.h"
#include <x86intrin.h>
#include <algorithm>
#include "xlf_util.h"

void read_buf_partial(void *addr, uint64_t max_size);
void trigger_prefetching(void *addr, uint64_t max_size);
void extreme_case_prefetching(void *addr, uint64_t max_size);
void read_after_flush(void *addr, uint64_t max_size);
void write_buffer(void *addr, uint64_t max_size);
void write_buffer_flushing_period(void *addr, uint64_t max_size);
void write_buffer_size(void *addr, uint64_t max_size);
void seperate_rd_wr_buf(void *addr, uint64_t max_size);
void read_and_write_bw(void *addr, uint64_t max_size);
void read_after_flush_lock_contention(void *addr, uint64_t max_size);
void access_lat(void *addr, uint64_t max_size);
void wr_lat_by_calc(void *addr, uint64_t max_size);
void wr_bw_by_calc(void *addr, uint64_t max_size);
void read_buf_amp_tst(void *addr, uint64_t max_size);
void lat_test(void *addr, uint64_t max_size);
void veri_rmw(void *addr, uint64_t max_size);

inline uint64_t rdtsc()
{
    unsigned int lo, hi;
    __asm__ __volatile__("rdtsc"
                         : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}
