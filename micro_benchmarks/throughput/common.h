#pragma once
#include "../cpu_info.h"
#include "../global.h"
// #define WSS_UNIT(i)  wss_##i##B_unit_t
// #define WSS_UNIT_T(i)                   \
//     struct wss_##i##B_unit_t            \
//     {                                   \
//         struct wss_##i##B_unit_t *next; \
//         uint64_t pad[i/8-1];                     \
//     }
// #define a 9

// #if a>10
// int b;
// #endif

// WSS_UNIT_T(64);
// WSS_UNIT_T(128);
// WSS_UNIT_T(256);
// WSS_UNIT_T(512);


// #define FUNCTION do{{
//     register int sum = 0;
//     int *lastone = (int *)(addr + size);
//     int *p;
//     while (iterations-- > 0)
//     {
//         for (auto offest : *sequence)
//         {
//             p = ((int *)addr) + (offest << 8);
// #define DOIT(i) volatile auto buff##i = _mm512_stream_load_si512((void *)(p + i));
// #include "op_1024B.h"
//         }
//     }
//     user_result_dummy += sum;
// }
// #undef DOIT}while(0)


// template<typename T>
// void frd(void *addr, uint64_t size, int iterations, std::vector<uint32_t> *);

void
frd_64B(void *addr, uint64_t size, int iterations, std::vector<uint32_t> *);
void frd_128B(void *addr, uint64_t size, int iterations, std::vector<uint32_t> *);
void frd_256B(void *addr, uint64_t size, int iterations, std::vector<uint32_t> *);
void frd_512B(void *addr, uint64_t size, int iterations, std::vector<uint32_t> *);
void frd_1024B(void *addr, uint64_t size, int iterations, std::vector<uint32_t> *);
void frd_2048B(void *addr, uint64_t size, int iterations, std::vector<uint32_t> *);
void frd_4096B(void *addr, uint64_t size, int iterations, std::vector<uint32_t> *);
void f64b_drain_wr(void *addr, uint64_t size, int iterations, std::vector<uint32_t> *);

void fwr_512B(void *addr, uint64_t size, int iterations, std::vector<uint32_t> *);