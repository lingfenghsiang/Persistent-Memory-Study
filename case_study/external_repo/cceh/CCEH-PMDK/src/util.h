#ifndef UTIL_H_
#define UTIL_H_

#include <cstdlib>
#include <cstdint>
#include <x86intrin.h>

#define CPU_FREQ_MHZ (1994) // cat /proc/cpuinfo
#define CAS(_p, _u, _v) (__atomic_compare_exchange_n (_p, _u, _v, false, __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE))
#define kCacheLineSize (64)

static inline void CPUPause(void){
    __asm__ volatile("pause":::"memory");
}

inline void mfence(void){
    _mm_sfence();
    // asm volatile("mfence":::"memory");
}

#endif
