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

        float imc_read, imc_write, media_read, media_write;
        {

            util::PmmDataCollector measure("DIMM data", &imc_read, &imc_write, &media_read, &media_write);
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
        std::cout << "-------result--------" << std::endl;
        std::cout << "[wss]:[" << ((sequence.size() * granularity_buf_line) << 8) << "](B)" << std::endl
                  << "[ideal read data]:[" << size * iterations / 1024 / 1024 << "](MB)" << std::endl
                  << "[imc read]:[" << imc_read << "](MB)" << std::endl
                  << "[granularity]:[" << granularity_buf_line * 256 << "](B)" << std::endl
                  << "[imc read ratio]:[" << imc_read / (size * iterations / 1024 / 1024) << "]" << std::endl
                  << "[media read]:[" << media_read << "](MB)" << std::endl
                  << "[pm read ratio]:["
                  << media_read / (size * iterations / 1024 / 1024) << "]"
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
