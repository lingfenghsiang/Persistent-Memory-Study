#include "common.h"

static inline void write_4_cacheline(char *working_addr, uint64_t i)
{
    working_addr[i + 0] = 0;
    working_addr[i + 64] = 0;
    working_addr[i + 128] = 0;
    working_addr[i + 192] = 0;
    _mm_clflush(&working_addr[i + 0]);
    _mm_clflush(&working_addr[i + 64]);
    _mm_clflush(&working_addr[i + 128]);
    _mm_clflush(&working_addr[i + 192]);
}

static inline void write_3_cacheline(char *working_addr, uint64_t i)
{
    working_addr[i + 0] = 0;
    working_addr[i + 64] = 0;
    working_addr[i + 128] = 0;
    // working_addr[i + 192] = 0;
    _mm_clflush(&working_addr[i + 0]);
    _mm_clflush(&working_addr[i + 64]);
    _mm_clflush(&working_addr[i + 128]);
    // _mm_clflush(&working_addr[i + 192]);
}

static inline void write_2_cacheline(char *working_addr, uint64_t i)
{
    working_addr[i + 0] = 0;
    working_addr[i + 64] = 0;
    // working_addr[i + 128] = 0;
    // working_addr[i + 192] = 0;
    _mm_clflush(&working_addr[i + 0]);
    _mm_clflush(&working_addr[i + 64]);
    // _mm_clflush(&working_addr[i + 128]);
    // _mm_clflush(&working_addr[i + 192]);
}

static inline void write_1_cacheline(char *working_addr, uint64_t i)
{
    working_addr[i + 0] = 0;
    // working_addr[i + 64] = 0;
    // working_addr[i + 128] = 0;
    // working_addr[i + 192] = 0;
    _mm_clflush(&working_addr[i + 0]);
    // _mm_clflush(&working_addr[i + 64]);
    // _mm_clflush(&working_addr[i + 128]);
    // _mm_clflush(&working_addr[i + 192]);
}

void write_buffer(void *addr, uint64_t max_size)
{
    int thread_num = 1;

    void (*write_xp_line)(char *working_addr, uint64_t i);
    auto func = [&](uint64_t wss, int iterations)
    {
        auto working_addr = (char *)addr;

        float media_rd, media_wr, imc_rd, imc_wr;
        {
            util::PmmDataCollector meansure("dimm", &imc_rd, &imc_wr, &media_rd, &media_wr);
            for (int j = 0; j < iterations; j++)
            {
                // you may put the offsets in a vector and std::shuffle it to test random write case,
                // the results would be the same
                for (uint64_t i = 0; i < wss; i += 256)
                {
                    (*write_xp_line)(working_addr, i);
                }
            }
        }
        std::cout << "-------result--------" << std::endl;
        if (write_xp_line == write_1_cacheline)
        {
            std::cout << "[write type]:[1/4XPline] " << std::endl;
        }
        else if (write_xp_line == write_2_cacheline)
        {
            std::cout << "[write type]:[2/4XPline] " << std::endl;
        }
        else if (write_xp_line == write_3_cacheline)
        {
            std::cout << "[write type]:[3/4XPline] " << std::endl;
        }
        else if (write_xp_line == write_4_cacheline)
        {
            std::cout << "[write type]:[4/4XPline] " << std::endl;
        }
        else
        {
            perror("write function");
            exit(EXIT_FAILURE);
        }
        std::cout << "[wss]:[" << wss << "] "
                  << "[WA]:[" << media_wr / imc_wr << "]" << std::endl;
        std::cout << "---------------------" << std::endl;
    };
    write_xp_line = write_1_cacheline;
    for (uint64_t wss = 256; wss <= 32768; wss += 256)
    {
        func(wss, (1ULL << 30) / wss);
    }
    write_xp_line = write_2_cacheline;
    for (uint64_t wss = 256; wss <= 32768; wss += 256)
    {
        func(wss, (1ULL << 30) / wss);
    }
    write_xp_line = write_3_cacheline;
    for (uint64_t wss = 256; wss <= 32768; wss += 256)
    {
        func(wss, (1ULL << 30) / wss);
    }
    write_xp_line = write_4_cacheline;
    for (uint64_t wss = 256; wss <= 32768; wss += 256)
    {
        func(wss, (1ULL << 30) / wss);
    }
}

static int user_result_dummy = 0;
void write_buffer_flushing_period(void *addr, uint64_t max_size)
{
    auto func = [&](uint64_t wss, int iterations, int counter_loop)
    {
        std::cout << "-------result--------" << std::endl;
        __m512i wr_buf;
        __m512i *base_addr = (__m512i *)addr;

        /**
         * @brief delayer is used to adjust the time interval between writes,
         * the corresponding delayed time varies as on different machines
         * 
         */
        volatile int delayer = 0;
        uint64_t cl_num = wss >> 6;
        uint64_t start_timer = 0, end_timer = 0;
        _mm_mfence();
        memset(&wr_buf, 0, sizeof(__m512i));
        start_timer = rdtsc();
        for (uint64_t i = 0; i < 100000; i++)
        {
            for (int k = 0; k < counter_loop; k++)
            {
                delayer += k;
            }
        }
        end_timer = rdtsc();
        std::cout << "[delay]:[" << (end_timer - start_timer) / 100000 << "](CPU cycle) ";
        float imc_rd, imc_wr, media_rd, media_wr;
        {
            util::PmmDataCollector measure("pmm", &imc_rd, &imc_wr, &media_rd, &media_wr);
            start_timer = rdtsc();
            for (uint64_t j = 0; j < iterations; j++)
            {
                for (uint64_t i = 0; i < cl_num; i++)
                {

                    _mm512_stream_si512(base_addr + i, wr_buf);
                    if ((i & 0x3) == 3)
                    {
                        for (int k = 0; k < counter_loop; k++)
                        {
                            delayer += k;
                        }
                    }
                }
                _mm_mfence();
            }
            end_timer = rdtsc();
        }

        user_result_dummy += delayer;
        std::cout << "[wss]:[" << wss << "] "
                  << "[count_loop]:[" << counter_loop << "] "
                  << "[period]:[" << (end_timer - start_timer) / iterations << "] "
                  << "[WA]:[" << media_wr / imc_wr << "]" << std::endl;
        std::cout << "---------------------" << std::endl;
    };
    for (uint64_t i = (1ULL << 12); i < (1ULL << 15); i = (i << 1))
    {
        for (int j = 0; j < 200; j += 10)
        {
            func(i, int((1ULL << 29) / i), j);
        }
    }

}
