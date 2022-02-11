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

    register int sum = 0;
    volatile int tmp = 0;
    auto func = [&](uint64_t size, int iterations, int granularity_buf_line)
    {
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
        std::vector<util::DimmObj> dimm_array;
        {

            util::PmmDataCollector measure("DIMM data", &dimm_array);
            for (int j = 0; j < iterations; j++)
            {
                std::random_shuffle(sequence.begin(), sequence.end());
                for (auto i : sequence)
                {
                    int *base_addr = (int *)(addr + ((i << 8) * granularity_buf_line));

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
                }
            }
            user_result_dummy += sum;
        }
        util::DimmObj *target_dimm = nullptr;
        for (auto &i : dimm_array)
        {
            if (!target_dimm)
            {
                target_dimm = &i;
            }
            if (target_dimm->imc_read < i.imc_read)
            {
                target_dimm = &i;
            }
        }
        std::cout << "-------result--------" << std::endl;
        std::cout << "[wss]:[" << ((sequence.size() * granularity_buf_line) << 8) << "](B)" << std::endl
                  << "[ideal read data]:[" << size * iterations / 1024 / 1024 << "](MB)" << std::endl
                  << "[imc read]:[" << target_dimm->imc_read << "](MB)" << std::endl
                  << "[granularity]:[" << granularity_buf_line * 256 << "](B)" << std::endl
                  << "[imc read ratio]:[" << target_dimm->imc_read / (size * iterations / 1024 / 1024) << "]" << std::endl
                  << "[media read]:[" << target_dimm->media_rd << "](MB)" << std::endl
                  << "[pm read ratio]:["
                  << target_dimm->media_rd / (size * iterations / 1024 / 1024) << "]"
                  << std::endl;
        std::cout << "---------------------" << std::endl;
    };

    for (int i = 1; i <= 4; i++)
    {
        for (uint64_t bits = 12; bits < 31; bits++)
        {
            func(1ULL << bits, 1ULL << (33 - bits), i);
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
    auto func = [&](uint64_t unit_num, int iterations, bool with_prefetch)
    {
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
        std::vector<util::DimmObj> dimm_array;
        register uint64_t latency = 0;
        {
            util::PmmDataCollector measure("DIMM data", &dimm_array);
            register int sum = 0;
            
            register working_unit_t *tmp_ptr;
            __m256i bit256_buf;
            working_unit_t *starting_ptr, *working_ptr;
            for (int i = 0; i < iterations; i++)
            {
                working_ptr = starting_ptr = working_unit_array + rand() % unit_num;
                for (;;)
                {
                    auto start = rdtsc();
                    // do tasks here
                    if (with_prefetch)
                    {
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
                    }
                    else
                    {
                        bit256_buf = _mm256_stream_load_si256((__m256i *)(working_ptr->cacheline0 + 24));
#define DOIT(i) sum += ((char *)(&bit256_buf))[i];
#include "./prefetching_work_case.h"
#undef DOIT
                        bit256_buf = _mm256_stream_load_si256((__m256i *)(working_ptr->cacheline1 + 32));
#define DOIT(i) sum += ((char *)(&bit256_buf))[i];
#include "./prefetching_work_case.h"
#undef DOIT
                        bit256_buf = _mm256_stream_load_si256((__m256i *)(working_ptr->cacheline2 + 32));
#define DOIT(i) sum += ((char *)(&bit256_buf))[i];
#include "./prefetching_work_case.h"
#undef DOIT
                        bit256_buf = _mm256_stream_load_si256((__m256i *)(working_ptr->cacheline3 + 32));
#define DOIT(i) sum += ((char *)(&bit256_buf))[i];
#include "./prefetching_work_case.h"
#undef DOIT
                    }
                    auto end = rdtsc();
                    latency += end - start;
                    tmp_ptr = working_ptr->next;
                    _mm_clflush(working_ptr->cacheline0);
                    _mm_clflush(working_ptr->cacheline1);
                    _mm_clflush(working_ptr->cacheline2);
                    _mm_clflush(working_ptr->cacheline3);
                    working_ptr = tmp_ptr;
                    if (working_ptr == starting_ptr)
                        break;
                }
            }
            user_result_dummy += sum;
        }

        util::DimmObj *target_dimm = nullptr;
        for (auto &i : dimm_array)
        {
            if (!target_dimm)
            {
                target_dimm = &i;
            }
            if (target_dimm->imc_read < i.imc_read)
            {
                target_dimm = &i;
            }
        }
        std::cout << "-------result--------" << std::endl;
        std::cout << "[wss]:[" << unit_num * sizeof(struct working_unit_t) << "](B)" << std::endl;
        std::cout << "[ideal read]:[" << unit_num * sizeof(struct working_unit_t) / (1ULL << 20) * iterations << "](MB)" << std::endl;
        std::cout << "[imc read]:[" << target_dimm->imc_read << "](MB)" << std::endl;
        std::cout << "[pm read]:[" << target_dimm->media_rd << "](MB)" << std::endl;
        std::cout << "[latency]:[" << latency / unit_num / iterations << "](CPU cycles)" << std::endl;
        if (with_prefetch)
        {
            std::cout << "[type]:[normal load]" << std::endl;
        }
        else
        {
            std::cout << "[type]:[nt cpy then load]" << std::endl;
        }
        std::cout << "---------------------" << std::endl;
    };
    for (int i = 12; i < 31; i++)
    {
        func(1ULL << (i - 8), 1ULL << (31 - i), true);
        func(1ULL << (i - 8), 1ULL << (31 - i), false);
    }
}