#include "common.h"

static int user_result_dummy = 0;
void seperate_rd_wr_buf(void *addr, uint64_t max_size)
{
    /// work type 1 for only wr, 2 for only rd, 3 for both mixed
    auto func = [&](int iterations, int work_type) {
        register int sum = 0;

        if (max_size <= (36ULL << 10))
        {
            perror("working set size too small");
            exit(EXIT_FAILURE);
        }
        char *rd_base = (char *)addr;
        char *wr_base = rd_base + (20ULL << 10);
        __m512i write_example;
        memset(&write_example, 1, 64);
        float imc_rd, imc_wr, media_rd, media_wr;
        {
            util::PmmDataCollector measure("PM data", &imc_rd, &imc_wr, &media_rd, &media_wr);
            for (uint64_t i = 0; i < iterations; i++)
            {
                // read operation
                if (work_type & 0x2)
                    for (uint64_t j = 0; j < (20ULL << 10); j += 256)
                    {
                        _mm_clflushopt(rd_base + j + 0);
                    }
                // write operation
                if (work_type & 0x1)
                    for (uint64_t j = 0; j < (13ULL << 10); j += 256)
                    {
                        _mm512_stream_si512((__m512i *)(wr_base + j + 0), write_example);
                    }
                if (work_type & 0x2)
                    for (uint64_t j = 0; j < (20ULL << 10); j += 256)
                    {
                        _mm_clflushopt(rd_base + j + 64);
                    }
                if (work_type & 0x1)
                    for (uint64_t j = 0; j < (13ULL << 10); j += 256)
                    {
                        _mm512_stream_si512((__m512i *)(wr_base + j + 64), write_example);
                    }
                if (work_type & 0x2)
                    for (uint64_t j = 0; j < (20ULL << 10); j += 256)
                    {
                        _mm_clflushopt(rd_base + j + 128);
                    }
                if (work_type & 0x1)
                    for (uint64_t j = 0; j < (13ULL << 10); j += 256)
                    {
                        // _mm_stream_pi((__m64 *)(wr_base + j + 128), (__m64)write_example);
                        _mm512_stream_si512((__m512i *)(wr_base + j + 128), write_example);
                    }
                if (work_type & 0x2)
                    for (uint64_t j = 0; j < (20ULL << 10); j += 256)
                    {
                        _mm_clflushopt(rd_base + j + 192);
                    }
            }
        }
        std::cout << "imc rd:" << imc_rd << std::endl
                  << "imc wr:" << imc_wr << std::endl
                  << "media rd:" << media_rd << std::endl
                  << "media wr:" << media_wr << std::endl
                  << std::endl;
    };
    // func(1ULL << 19, 1);
    // func(1ULL << 19, 2);
    // func(1ULL << 19, 3);

    /**
     * @brief this function serves to insert write among reads, to see the critical requirements of transition between read buffer and write buffer
     * 
     */
    auto func1 = [&](int local_wss, uint64_t iterations, uint64_t distance)
    {
        char *rd_base = (char *)addr;
        float imc_rd, imc_wr, media_rd, media_wr;
        __m512i write_example;
        register int sum = 0;
        memset(&write_example, 1, 64);
        std::vector<util::DimmObj> dimm_data;
        {
            util::PmmDataCollector measure("PM data", &dimm_data);
            // for (uint64_t j = 0; (j < local_wss) ; j += 256)
            //     {
            //         _mm512_stream_si512((__m512i *)(rd_base + j + 192), write_example);
            //     }
            for (uint64_t i = 0; i < iterations; i++)
            {
                if ((i % distance) == 0)
                {
                    for (uint64_t j = 0; j < local_wss; j += 256)
                    {
                        // _mm512_stream_si512((__m512i *)(rd_base + j +192), write_example);
                        rd_base[j + 192] = 0;
                        _mm_clflush(rd_base + j + 192);
                    }
                    // for (uint64_t j = 0; j < local_wss; j += 256)
                    // {
                    //     _mm512_stream_si512((__m512i *)(rd_base + j + 64), write_example);
                    // }
                    // for (uint64_t j = 0; j < local_wss; j += 256)
                    // {
                    //     _mm512_stream_si512((__m512i *)(rd_base + j + 128), write_example);
                    // }
                    // for (uint64_t j = 0; j < local_wss; j += 256)
                    // {
                    //     _mm512_stream_si512((__m512i *)(rd_base + j + 192), write_example);
                    // }
                }


                for (uint64_t j = 0; j < local_wss; j += 256)
                {
                    _mm_clflush(rd_base + j + 0);
                    // sum+=rd_base[j + 0];
                }

                
                   

                for (uint64_t j = 0; j < local_wss; j += 256)
                {
                    _mm_clflush(rd_base + j + 64);
                    //  sum+=rd_base[j + 64];
                }
                for (uint64_t j = 0; j < local_wss; j += 256)
                {
                    _mm_clflush(rd_base + j + 128);
                    //  sum+=rd_base[j + 128];
                }
            }
        }
        // user_result_dummy += sum;
        std::cout << "-------result--------" << std::endl;
        for(auto target:dimm_data)
        {
            if(target.dimm_id_.compare("0x0100") == 0)
                std::cout << "[distance]:[" << distance << "], [media_rd]:[" << target.media_rd << "], [ratio]:["
                          << ((local_wss / 1024 * iterations * 1.0) / 1024) / target.media_rd << "]" << std::endl;
        }
        
        std::cout << "---------------------" << std::endl;
    };
    int tmp_distance = 0;
    for (uint64_t k = 6; k < 12; k++)
    {
        tmp_distance = 0;
        for (int i = 0; i < 10; i++)
        {
            // tmp_distance += tmp_distance;
            for (int j = 1; j < 4; j++)
            {
                tmp_distance += (1ULL << i);
                func1((k << 10), 1ULL << 16, tmp_distance);
            }
            // func1((8ULL << 10), 1ULL << 20, i*4);
        }
        std::cout<< "**********************************************" <<std::endl;
    }

    // func1((8ULL << 10), 1ULL << 20, 21);
    // func1((8ULL << 10), 1ULL << 20, 33);
    // func1((8ULL << 10), 1ULL << 20, 34);
    // func1((8ULL << 10), 1ULL << 20, 35);
    // func1((8ULL << 10), 1ULL << 20, 36);
}