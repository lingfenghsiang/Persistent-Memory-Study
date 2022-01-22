#include "common.h"

static int user_result_dummy = 0;
/**
 * @brief in this test, we set the write working set size as 11KB, 
 * 
 * @param addr 
 * @param max_size 
 */
void seperate_rd_wr_buf(void *addr, uint64_t max_size)
{
    /// work type 1 for only wr, 2 for only rd, 3 for both mixed
    auto func = [&](int iterations, int work_type)
    {
        register int sum = 0;

        if (max_size <= (36ULL << 10))
        {
            perror("working set size too small");
            exit(EXIT_FAILURE);
        }
        char *rd_base = (char *)addr;
        char *wr_base = rd_base + (16ULL << 10);
        __m512i write_example;
        memset(&write_example, 1, 64);
        float imc_rd, imc_wr, media_rd, media_wr;
        {
            util::PmmDataCollector measure("PM data", &imc_rd, &imc_wr, &media_rd, &media_wr);
            for (uint64_t i = 0; i < iterations; i++)
            {
                // read operation
                if (work_type & 0x2)
                    for (uint64_t j = 0; j < (16ULL << 10); j += 256)
                    {
                        _mm_clflushopt(rd_base + j + 0);
                    }
                // write operation
                if (work_type & 0x1)
                    for (uint64_t j = 0; j < (8ULL << 10); j += 256)
                    {
                        _mm512_stream_si512((__m512i *)(wr_base + j + 0), write_example);
                    }
                if (work_type & 0x2)
                    for (uint64_t j = 0; j < (16ULL << 10); j += 256)
                    {
                        _mm_clflushopt(rd_base + j + 64);
                    }
                if (work_type & 0x1)
                    for (uint64_t j = 0; j < (8ULL << 10); j += 256)
                    {
                        _mm512_stream_si512((__m512i *)(wr_base + j + 64), write_example);
                    }
                if (work_type & 0x2)
                    for (uint64_t j = 0; j < (16ULL << 10); j += 256)
                    {
                        _mm_clflushopt(rd_base + j + 128);
                    }
                if (work_type & 0x1)
                    for (uint64_t j = 0; j < (8ULL << 10); j += 256)
                    {
                        // _mm_stream_pi((__m64 *)(wr_base + j + 128), (__m64)write_example);
                        _mm512_stream_si512((__m512i *)(wr_base + j + 128), write_example);
                    }
                if (work_type & 0x2)
                    for (uint64_t j = 0; j < (16ULL << 10); j += 256)
                    {
                        _mm_clflushopt(rd_base + j + 192);
                    }
            }
        }
        std::cout << "-------result--------" << std::endl;
        if (work_type == 2)
        {
            std::cout << "[work type]:[pure read in 16KB working set size]" << std::endl;
        }
        else if (work_type == 1)
        {
            std::cout << "[work type]:[pure write in 8KB working set size]" << std::endl;
        }
        else if (work_type == 3)
        {
            std::cout << "[work type]:[interleaved read(in 16KB working set size) and write(in 8KB working set size)]" << std::endl;
        }
        std::cout << "[imc rd]:[" << imc_rd << "]" << std::endl
                  << "[imc wr]:[" << imc_wr << "]" << std::endl
                  << "[media rd]:[" << media_rd << "]" << std::endl
                  << "[media wr]:[" << media_wr << "]" << std::endl
                  << std::endl;
        std::cout << "---------------------" << std::endl;
    };
    func(1ULL << 19, 1);
    func(1ULL << 19, 2);
    func(1ULL << 19, 3);
}