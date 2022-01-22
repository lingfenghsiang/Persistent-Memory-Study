#include "common.h"

void write_buffer(void *addr, uint64_t max_size)
{
    int thread_num =30;
    pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, NULL, thread_num);
    auto func = [&](uint64_t wss, int iterations) {
        pthread_barrier_wait(&barrier);
        auto working_addr = (char *)addr;
        __m128i sse_buf;
        register uint64_t rand_offset = 0;
        memset(&sse_buf, 0, 16);
        {
            
            for (int j = 0; j < iterations; j++)
            {
                rand_offset = ((rand() << 9) & (wss - 1));
                for (uint64_t i = 0; i < wss; i += 512)
                {
                    _mm_stream_si128((__m128i *)(working_addr + ((i + 0 + rand_offset) & (wss - 1))), sse_buf);
                    _mm_stream_si128((__m128i *)(working_addr + ((i + 16 + rand_offset) & (wss - 1))), sse_buf);
                    _mm_stream_si128((__m128i *)(working_addr + ((i + 32 + rand_offset) & (wss - 1))), sse_buf);
                    _mm_stream_si128((__m128i *)(working_addr + ((i + 48 + rand_offset) & (wss - 1))), sse_buf);

                    _mm_stream_si128((__m128i *)(working_addr + ((i + 64 + rand_offset) & (wss - 1))), sse_buf);
                    _mm_stream_si128((__m128i *)(working_addr + ((i + 80 + rand_offset) & (wss - 1))), sse_buf);
                    _mm_stream_si128((__m128i *)(working_addr + ((i + 96 + rand_offset) & (wss - 1))), sse_buf);
                    _mm_stream_si128((__m128i *)(working_addr + ((i + 112 + rand_offset) & (wss - 1))), sse_buf);

                    _mm_stream_si128((__m128i *)(working_addr + ((i + 128 + rand_offset) & (wss - 1))), sse_buf);
                    _mm_stream_si128((__m128i *)(working_addr + ((i + 144 + rand_offset) & (wss - 1))), sse_buf);
                    _mm_stream_si128((__m128i *)(working_addr + ((i + 160 + rand_offset) & (wss - 1))), sse_buf);
                    _mm_stream_si128((__m128i *)(working_addr + ((i + 176 + rand_offset) & (wss - 1))), sse_buf);

                    // _mm_stream_si128((__m128i *)(working_addr + ((i + 192+ rand_offset) & (wss - 1))), sse_buf);
                    // _mm_stream_si128((__m128i *)(working_addr + ((i + 208+ rand_offset) & (wss - 1))), sse_buf);
                    // _mm_stream_si128((__m128i *)(working_addr +(( i + 224+ rand_offset) & (wss - 1))), sse_buf);
                    // _mm_stream_si128((__m128i *)(working_addr + ((i + 240+ rand_offset) & (wss - 1))), sse_buf);

                    _mm_stream_si128((__m128i *)(working_addr + ((i + 256 + rand_offset) & (wss - 1))), sse_buf);
                    _mm_stream_si128((__m128i *)(working_addr + ((i + 272 + rand_offset) & (wss - 1))), sse_buf);
                    _mm_stream_si128((__m128i *)(working_addr + ((i + 288 + rand_offset) & (wss - 1))), sse_buf);
                    _mm_stream_si128((__m128i *)(working_addr + ((i + 304 + rand_offset) & (wss - 1))), sse_buf);

                    // working_addr[i + 0] = 0;
                    // working_addr[i + 64] = 0;
                    // working_addr[i + 128] = 0;
                    // // working_addr[i + 192] = 0;
                    // _mm_clflush(&working_addr[i + 0]);
                    // _mm_clflush(&working_addr[i + 64]);
                    // _mm_clflush(&working_addr[i + 128]);
                    // // _mm_clflush(&working_addr[i + 192]);
                }
            }
        }
    };
    // func(8192, 1ULL<<22);
    std::vector<std::thread> work_group;
    {
        util::PmmDataCollector meansure("dimm");
        for (int i = 0; i < thread_num; i++)
        {
            work_group.push_back(std::thread(func,32768, 1ULL << 20));
        }
        for (int i = 0; i < thread_num; i++)
        {
            work_group.at(i).join();
        }
    }

    // func(6144, 1ULL<<22);
}

static int user_result_dummy = 0;
void write_buffer_flushing_period(void *addr, uint64_t max_size)
{
    auto func = [&](uint64_t wss, int iterations, int counter_loop)
    {
        std::cout << "-------result--------" << std::endl;
        __m512i wr_buf;
        __m512i *base_addr = (__m512i *)addr;
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
    // func(4800, 1000000, 0);
}

void write_buffer_size(void *addr, uint64_t max_size)
{
    auto func = [&](uint64_t wss, int iterations, bool random)
    {
        std::cout << "-------result--------" << std::endl;
        __m512i wr_buf;
        __m512i *base_addr = (__m512i *)addr;
        char* work_ptr = (char*)addr;
        _mm_mfence();
        memset(&wr_buf, 0, sizeof(__m512i));
        std::vector<uint64_t> order(wss / 256);
        for (uint64_t i = 0; i < wss / 256; i++)
        {
            order.at(i) = i * 256;
        }
        if (random)
        {
            std::random_shuffle(order.begin(), order.end());
        }
        std::vector<util::DimmObj> dimm_array;
        {
            util::PmmDataCollector measure("pmm", &dimm_array);

            for (uint64_t i = 0; i < iterations; i++)
            {
                for (auto j : order)
                {
                    _mm512_stream_si512((__m512i *)(work_ptr + j + 0), wr_buf);
                    // _mm512_stream_si512((__m512i *)(work_ptr + j + 64), wr_buf);
                    // _mm512_stream_si512((__m512i *)(work_ptr + j + 128), wr_buf);
                    // _mm512_stream_si512((__m512i *)(work_ptr + j + 192), wr_buf);
                }
                // for (uint64_t j = 0; j < wss; j += 256)
                // {
                //     _mm512_stream_si512((__m512i *)(work_ptr + j + 128), wr_buf);
                //     _mm512_stream_si512((__m512i *)(work_ptr + j + 192), wr_buf);
                // }
            }
        }
        for (auto i : dimm_array)
        {
            if (i.dimm_id_.compare("0x0001") == 0)
            {
                std::cout << "[wss]:[" << wss << "] "
                << "[type]:[" << "0.5wr" << "] "
                          << "[WA]:[" << i.media_wr / i.imc_wr << "]" << std::endl;
                break;
            }
        }

        std::cout << "---------------------" << std::endl;
    };
    for (uint64_t i = (1ULL << 9); i < 32768; i +=256)
    {

            func(i, ((1ULL << 32) / i), true);

    }
}

/**
 * @brief to verify when the wss is small, whether the data is read-modify-write
 * on pm, the wss need to be slightly larger than read/write buffer size and the
 * data should be written in small granularity, say 64B
 * 
 * @param addr 
 * @param max_size 
 */
void veri_rmw(void *addr, uint64_t max_size)
{
    __m512i cl_buffer;
    memset(&cl_buffer, 0, sizeof(cl_buffer));
    auto func = [&](uint64_t wss, uint64_t iteration)
    {
        std::cout << "-------result--------" << std::endl;
        char *work_ptr = (char *)addr;
        std::vector<uint64_t> order(wss/256);
        for (uint64_t i = 0; i < (wss / 256); i++)
        {
            order.at(i) = i * 256;
        }
        std::random_shuffle(order.begin(), order.end());
        std::vector<util::DimmObj> dimm_info;
        {
            util::PmmDataCollector measure("dimm", &dimm_info);
            for (uint64_t i = 0; i < iteration; i++)
            {
                for (auto j : order)
                {
                    _mm512_stream_si512((__m512i *)(work_ptr + j), cl_buffer);
                }
            }
        }
        for (auto i : dimm_info)
        {
            if (i.dimm_id_.compare("0x0100") == 0)
            {
                std::cout << "[wss]:[" << wss << "] "
                          << "[type]:["
                          << "0.5wr"
                          << "] "
                          << "[WA]:[" << i.media_wr / i.imc_wr << "]" << std::endl;
                break;
            }
        }
        std::cout << "---------------------" << std::endl;
    };
    func((1ULL << 29), 4);
};