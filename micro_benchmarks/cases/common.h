#pragma once
#include "../global.h"
#include <x86intrin.h>
#include <algorithm>
#include "pm_util.h"

void read_buf_amp_tst(void *addr, uint64_t max_size);                  // 0
void trigger_prefetching(void *addr, uint64_t max_size);               // 1
void write_buffer(void *addr, uint64_t max_size);                      // 2
void write_buffer_flushing_period(void *addr, uint64_t max_size);      // 3
void seperate_rd_wr_buf(void *addr, uint64_t max_size);                // 4
void read_after_flush(void *addr, uint64_t max_size);                  // 5
void access_lat(void *addr, uint64_t max_size);                        // 6
void read_after_flush_lock_contention(void *addr, uint64_t max_size);  // 7
void extreme_case_prefetching(void *addr, uint64_t max_size);          // 8
void rd_throughput_against_prefetching(void *addr, uint64_t max_size); // 9

inline uint64_t rdtsc()
{
    unsigned int lo, hi;
    __asm__ __volatile__("rdtsc"
                         : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}
