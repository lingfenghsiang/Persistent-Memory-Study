/*
 * @Author: your name
 * @Date: 2020-12-31 02:43:58
 * @LastEditTime: 2021-01-15 10:32:39
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /throughput/experiments_sets.cpp
 */

#include "./common.h"
#include <x86intrin.h>

int user_result_dummy;

void frd_4096B(void *addr, uint64_t size, int iterations, std::vector<uint32_t> *sequence)
{
    register int sum = 0;
    int *lastone = (int *)(addr + size);
    int *p;
    while (iterations-- > 0)
    {
        for (auto offest : *sequence)
        {
            p = ((int *)addr) + (offest << 10);
#define DOIT(i) volatile auto buff##i = _mm512_stream_load_si512((void *)(p + i));
#include "op_4096B.h"
        }
    }
    user_result_dummy += sum;
}

void frd_2048B(void *addr, uint64_t size, int iterations, std::vector<uint32_t> *sequence)
{
    register int sum = 0;
    int *lastone = (int *)(addr + size);
    int *p;
    while (iterations-- > 0)
    {
        for (auto offest : *sequence)
        {
            p = ((int *)addr) + (offest << 9);
#define DOIT(i) volatile auto buff##i = _mm512_stream_load_si512((void *)(p + i));
#include "op_2048B.h"
        }
    }
    user_result_dummy += sum;
}
#undef DOIT

void frd_1024B(void *addr, uint64_t size, int iterations, std::vector<uint32_t> *sequence)
{
    register int sum = 0;
    int *lastone = (int *)(addr + size);
    int *p;
    while (iterations-- > 0)
    {
        for (auto offest : *sequence)
        {
            p = ((int *)addr) + (offest << 8);
#define DOIT(i) volatile auto buff##i = _mm512_stream_load_si512((void *)(p + i));
#include "op_1024B.h"
        }
    }
    user_result_dummy += sum;
}
#undef DOIT

void frd_512B(void *addr, uint64_t size, int iterations, std::vector<uint32_t> *sequence)
{
    register int sum = 0;
    int *lastone = (int *)(addr + size);
    int *p;
    while (iterations-- > 0)
    {
        for (auto offest : *sequence)
        {
            p = ((int *)addr) + (offest << 7);
#define DOIT(i) volatile auto buff##i = _mm512_stream_load_si512((void *)(p + i));
#include "op_512B.h"
        }
    }
    user_result_dummy += sum;
}
#undef DOIT

void frd_256B(void *addr, uint64_t size, int iterations, std::vector<uint32_t> *sequence)
{
    register int sum = 0;
    int *lastone = (int *)(addr + size);
    int *p;
    while (iterations-- > 0)
    {
        for (auto offest : *sequence)
        {
            p = ((int *)addr) + (offest << 6);
#define DOIT(i) volatile auto buff##i = _mm512_stream_load_si512((void *)(p + i));
#include "op_256B.h"
        }
    }
    user_result_dummy += sum;
}
#undef DOIT

void frd_128B(void *addr, uint64_t size, int iterations, std::vector<uint32_t> *sequence)
{
    register int sum = 0;
    int *lastone = (int *)(addr + size);
    int *p;
    while (iterations-- > 0)
    {
        for (auto offest : *sequence)
        {
            p = ((int *)addr) + (offest << 5);
#define DOIT(i) volatile auto buff##i = _mm512_stream_load_si512((void *)(p + i));
#include "op_128B.h"
        }
    }
    user_result_dummy += sum;
}
#undef DOIT

void frd_64B(void *addr, uint64_t size, int iterations, std::vector<uint32_t> *sequence)
{
    register int sum = 0;
    __m512 the_buf;
    memset(&the_buf, 0, sizeof(__m512));
    int *lastone = (int *)(addr + size);
    int *p;
    while (iterations-- > 0)
    {
        for (auto offest : *sequence)
        {
            //  volatile auto buff##i=  _mm512_stream_load_si512 ((void *) (p+i));
            // _mm512_stream_ps ((float*) (p+i), buf);
            p = ((int *)addr) + (offest << 4);
#define DOIT(i) _mm512_stream_ps((float *)(p + i), the_buf);
#include "op_64B.h"
        }
    }
    user_result_dummy += sum;
}
#undef DOIT

void f64b_drain_wr(void *addr, uint64_t size, int iterations, std::vector<uint32_t> *sequence)
{
    register int sum = 0;
    int *lastone = (int *)(addr + size);

    while (iterations-- > 0)
    {
        int *p = (int *)addr;
        while (p < lastone)
        {
            p[0] = iterations;
            _mm_clwb(&p[0]);
            // _mm_sfence();
            p += 16;
        }
    }
    user_result_dummy += sum;
}
#undef DOIT

// void fwr_512B(void *addr, uint64_t size, int iterations, std::vector<uint32_t> *sequence)
// {
//     register int sum = 0;
//     __m512 the_buf;
//     memset(&the_buf, 0, sizeof(__m512));
//     int *lastone = (int *)(addr + size);
//     int *p;
//     while (iterations-- > 0)
//     {
//         for (auto offest : *sequence)
//         {
//             p = ((int *)addr) + (offest << 4);
// #define DOIT(i) p[i] = 1;
// #include "op_1_128.h"
//         }
//     }
//     user_result_dummy += sum;
// }
// #undef DOIT

void fwr_512B(void *addr, uint64_t size, int iterations, std::vector<uint32_t> *sequence)
{
    register int sum = 0;
    __m512 the_buf;
    memset(&the_buf, 0, sizeof(__m512));
    int *lastone = (int *)(addr + size);
    int *p = (int *)addr;
    while (iterations-- > 0)
    {
        while (p < lastone)
        {
#define DOIT(i) p[i] = 1;
#include "op_1_128.h"
            p += 128;
        }
//         for (int i = 0; i < sequence->size(); i++)
//         {
//             p = ((int *)addr) + (sequence->at(i) << 4);
//             #define DOIT(i) p[i] = 1;
// #include "op_1_128.h"
//             // p = ((int *)addr) + (offest << 4);
//         }
    }
    user_result_dummy += sum;
}
#undef DOIT