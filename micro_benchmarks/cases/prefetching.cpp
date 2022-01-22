#include "common.h"


static int user_result_dummy = 0;
/**
 * @brief notion: if you are running this function please turn off any hardware prefetching
 * this could be done in BIOS configurations
 * 
 * @param addr 
 * @param max_size 
 */
void trigger_prefetching(void *addr, uint64_t max_size)
{
    util::debug_perf_ppid();

    register int sum = 0;
    volatile int tmp = 0;
    auto func = [&](uint64_t size, int iterations, int granularity_buf_line) {
        if (size > max_size)
        {
            perror("too large wss");
            return;
        }

        // each sequence element is 256B
        std::vector<uint32_t> sequence((size >> 8) / granularity_buf_line);
        for (int i = 0; i < sequence.size(); i++)
        {
            sequence.at(i) = i;
        }
        std::srand(time(NULL));
        // std::shuffle(sequence.begin(), sequence.end(), rand());

        float imc_read, imc_write, media_read, media_write;
        {
            util::debug_perf_switch();
            util::PmmDataCollector measure("DIMM data", &imc_read, &imc_write, &media_read, &media_write);
            for (int j = 0; j < iterations; j++)
            {
                std::random_shuffle(sequence.begin(), sequence.end());
                for (auto i : sequence)
                {
                    int *base_addr = (int *)(addr + ((i << 8) * granularity_buf_line));
// #define DOIT(i) sum += base_addr[i];
// #include "ops_1_64.h"
// #undef DOIT
                    // sum += base_addr[60];
                    // sum += base_addr[61];
                    // sum += base_addr[63];
                    // #define COMP_GRANU(i)                  \
//     {                                  \
//         if (granularity_buf_line == i) \
//             goto flush;                  \
//     }
                    // #define DOIT(i) sum += base_addr[i];
                    // #include "./ops.h"
                    // #undef DOIT
                    // #undef COMP_GRANU
                    // flush:

#define COMP_GRANU(i)                  \
    {                                  \
        if (granularity_buf_line == i) \
            goto exit;                 \
    }

#define DOIT(i)                                                                           \
    sum += base_addr[i + 0] + base_addr[i + 1] + base_addr[i + 2] + base_addr[i + 3] +    \
           base_addr[i + 4] + base_addr[i + 5] + base_addr[i + 6] + base_addr[i + 7] +    \
           base_addr[i + 8] + base_addr[i + 9] + base_addr[i + 10] + base_addr[i + 11] +  \
           base_addr[i + 12] + base_addr[i + 13] + base_addr[i + 14] + base_addr[i + 15]; \
    _mm_clflush(base_addr + i);
#include "./op_flush.h"
#undef DOIT
#undef COMP_GRANU
                exit:
                    sum++;
                    // auto base_addr = addr + (i << 8);
                    // sum += *(int *)(base_addr +0);//for(int k = 0;k< 300;k++){user_result_dummy++;tmp++;}
                    // // sum += *(int *)(base_addr + 2);//for(int k = 0;k< 300;k++){user_result_dummy++;tmp++;}
                    // // sum += *(int *)(base_addr + 4);//for(int k = 0;k< 300;k++){user_result_dummy++;tmp++;}
                    // // sum += *(int *)(base_addr + 4);
                    // // sum += *(int *)(base_addr + 4);
                    // // sum += *(int *)(base_addr + 4);
                    // // sum += *(int *)(base_addr + 4);//for(int k = 0;k< 300;k++){user_result_dummy++;tmp++;}
                    // _mm_mfence();
                    // sum += *(int *)(base_addr + 64);//for(int k = 0;k< 300;k++){user_result_dummy++;tmp++;}
                    // // sum += *(int *)(base_addr +  66);//for(int k = 0;k< 300;k++){user_result_dummy++;tmp++;}
                    // // sum += *(int *)(base_addr +  64);//for(int k = 0;k< 300;k++){user_result_dummy++;tmp++;}
                    // // sum += *(int *)(base_addr + 64);
                    // // sum += *(int *)(base_addr +  64);
                    // // sum += *(int *)(base_addr +  64);//for(int k = 0;k< 300;k++){user_result_dummy++;tmp++;}
                    // _mm_mfence();
                    // sum += *(int *)(base_addr + 128);//for(int k = 0;k< 300;k++){user_result_dummy++;tmp++;}
                    // // sum += *(int *)(base_addr + 130);//for(int k = 0;k< 300;k++){user_result_dummy++;tmp++;}
                    // // sum += *(int *)(base_addr + 128);//for(int k = 0;k< 300;k++){user_result_dummy++;tmp++;}
                    // // sum += *(int *)(base_addr + 128);
                    // // sum += *(int *)(base_addr + 128);
                    // // sum += *(int *)(base_addr + 128);//for(int k = 0;k< 300;k++){user_result_dummy++;tmp++;}
                    // _mm_mfence();
                    // sum += *(int *)(base_addr + 192);//for(int k = 0;k< 300;k++){user_result_dummy++;tmp++;}
                    // // sum += *(int *)(base_addr + 194);//for(int k = 0;k< 300;k++){user_result_dummy++;tmp++;}
                    // // sum += *(int *)(base_addr + 192);//for(int k = 0;k< 300;k++){user_result_dummy++;tmp++;}
                    // // sum += *(int *)(base_addr + 192);
                    // // sum += *(int *)(base_addr +192);
                    // // sum += *(int *)(base_addr + 192);//for(int k = 0;k< 300;k++){user_result_dummy++;tmp++;}

                    // // _mm_mfence();
                    // for(int k = 0;k< 100;k++){user_result_dummy++;tmp++;}
                    // // _mm_mfence();
                    // _mm_clflush(base_addr + 0);
                    // _mm_clflush(base_addr + 64);
                    // _mm_clflush(base_addr + 128);
                    // _mm_clflush(base_addr + 192);
                }
            }
            user_result_dummy += sum + tmp;
            util::debug_perf_switch();
        }
        std::cout << "-------result--------" << std::endl;
        std::cout << "[wss]:[" << ((sequence.size() * granularity_buf_line) << 8) << "](B)" << std::endl
                  << "[ideal written data]:[" << size * iterations / 1024 / 1024 << "](MB)" << std::endl
                  << "[imc read]:[" << imc_read << "](MB)" << std::endl
                  << "[granularity]:[" << granularity_buf_line * 256 << "](B)" << std::endl
                  << "[media read]:[" << media_read << "](MB)" << std::endl;
        std::cout << "---------------------" << std::endl;
    };
    // for (int i = 28; i < 31; i++)
    // {
    //     uint64_t size = 1ULL << i;
    //     int iterations = (1ULL << (33 - i));
    //     func(size, iterations, 4);
    // }

    for (int i = 1; i <= 4; i++)
    {
        // uint64_t size = 1ULL << 30;
        // int iterations = (1ULL << (33 - 30));
        for (uint64_t m = 12; m < 31; m++)
        {
            func(1ULL << m, 1ULL << (33 - m), i);
        }
    }
}

struct working_unit_t
{
    struct working_unit_t *next;
    char cacheline0[56];
    // char pad[64];
    char cacheline1[64];
    char cacheline2[64];
    char cacheline3[64];
};

void extreme_case_prefetching(void *addr, uint64_t max_size)
{
    auto func = [&](uint64_t unit_num, int iterations) {
        if (unit_num * sizeof(working_unit_t) > max_size)
        {
            perror("working set size too large");
            exit(EXIT_FAILURE);
        }
        if (unit_num <= 1)
        {
            perror("working set size too small");
            exit(EXIT_FAILURE);
        }
        // shuffle the order to access the units
        std::vector<int> access_order(unit_num);
        for (int i = 0; i < unit_num; i++)
        {
            access_order.at(i) = i;
        }
        srand(time(NULL));
        std::random_shuffle(access_order.begin(), access_order.end());
        working_unit_t *working_unit_array = (working_unit_t *)addr;
        for (int i = 1; i < unit_num; i++)
        {
            working_unit_array[access_order.at(i - 1)].next = &working_unit_array[access_order.at(i)];
        }
        working_unit_array[access_order.at(unit_num - 1)].next = &working_unit_array[access_order.at(0)];
        _mm_mfence();
        {
            register int sum = 0;
            __m256i bit256_buf;
            util::PmmDataCollector measure("dimm data");
            working_unit_t *starting_ptr, *working_ptr;
            for (int i = 0; i < iterations; i++)
            {
                working_ptr = starting_ptr = working_unit_array + rand() % unit_num;
                for (;;)
                {
                    auto next = 
                    // do tasks here
                    #define DOIT(i) sum += working_ptr->cacheline0[i];
                    #include "./prefetching_work_case.h"
                    #undef DOIT

                    #define DOIT(i) sum += working_ptr->cacheline1[i];
                    #include "./prefetching_work_case.h"
                    #undef DOIT

                    #define DOIT(i) sum += working_ptr->cacheline2[i];
                    #include "./prefetching_work_case.h"
                    #undef DOIT

                    #define DOIT(i) sum += working_ptr->cacheline3[i];
                    #include "./prefetching_work_case.h"
                    #undef DOIT

                    #define DOIT(i) sum += working_ptr->cacheline3[i+64];
                    #include "./prefetching_work_case.h"
                    #undef DOIT
                    // bit256_buf = _mm256_stream_load_si256((__m256i *)(working_ptr->cacheline0 + 24));
                    // #define DOIT(i) sum += ((char*)(&bit256_buf))[i];
                    // #include "./prefetching_work_case.h"
                    // #undef DOIT
                    // bit256_buf = _mm256_stream_load_si256((__m256i *)(working_ptr->cacheline1 + 32));
                    // #define DOIT(i) sum += ((char*)(&bit256_buf))[i];
                    // #include "./prefetching_work_case.h"
                    // #undef DOIT
                    // bit256_buf = _mm256_stream_load_si256((__m256i *)(working_ptr->cacheline2 + 32));
                    // #define DOIT(i) sum += ((char*)(&bit256_buf))[i];
                    // #include "./prefetching_work_case.h"
                    // #undef DOIT
                    // bit256_buf = _mm256_stream_load_si256((__m256i *)(working_ptr->cacheline3 + 32));
                    // #define DOIT(i) sum += ((char*)(&bit256_buf))[i];
                    // #include "./prefetching_work_case.h"
                    // #undef DOIT

                    // _mm_clflush(working_ptr->cacheline0);
                    // _mm_clflush(working_ptr->cacheline1);
                    // _mm_clflush(working_ptr->cacheline2);
                    // _mm_clflush(working_ptr->cacheline3);
                    working_ptr = working_ptr->next;
                    if (working_ptr == starting_ptr)
                        break;
                }
            }
            user_result_dummy += sum;
        }
    };

    func(1ULL << 22, 10);
}