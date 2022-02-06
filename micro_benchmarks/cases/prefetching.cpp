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
        std::vector<uint32_t> sequence((size >> 6) / granularity_buf_line);
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
                    int *base_addr = (int *)(addr + ((i << 6) * granularity_buf_line));

#define COMP_GRANU(i)                  \
    {                                  \
        if (granularity_buf_line == i) \
            goto exit;                 \
    }

#define DOIT(i)          \
    sum += base_addr[i]; \
    _mm_clflush(base_addr + i);
#include "./flush.h"
#undef DOIT
#undef COMP_GRANU
                exit:
                    sum++;
                }
            }
            user_result_dummy += sum;
        }
        util::DimmObj *target_dimm=nullptr;
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

    for (int i = 4; i <= 4; i++)
    {
        for (uint64_t bits = 12; bits < 31; bits++)
        {
            func(1ULL << bits, 1ULL << (33 - bits), i);
        }
    }
}
