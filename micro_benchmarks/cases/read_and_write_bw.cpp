#include "common.h"


static int user_result_dummy = 0;

void read_and_write_bw(void *addr, uint64_t max_size)
{
    pthread_barrier_t barrier;
    std::vector<uint64_t> results;
    std::atomic<int> tid;

    auto rd_func = [&](void *local_addr, uint64_t working_set_size, int iterations) {
        register int sum = 0;
        int *lastone = (int *)(local_addr + working_set_size);
        uint64_t iter = iterations;
        int thread_id = tid.fetch_add(1);
        int *p;
        pthread_barrier_wait(&barrier);
        auto start_timer = std::chrono::system_clock::now();
        while (iterations-- > 0)
        {

            p = ((int *)local_addr);
            while (p < lastone)
            {
                // volatile auto buff##i = _mm512_stream_load_si512((void *)(p + i)); sum +=p[i];_mm_clwb(p + i);
                //  256B       granularity
#define DOIT(i) volatile auto buff##i = _mm512_stream_load_si512((void *)(p + i));
#include "op_4_flush.h"
#undef DOIT
                p += 64;
            }
            // user_result_dummy += sum;
        }
        auto end_timer = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_timer - start_timer);
        results.at(thread_id) = duration.count();
        
    };
    auto wr_func = [&](void *local_addr, uint64_t working_set_size, int iterations)
    {
        int *lastone = (int *)(local_addr + working_set_size);
        uint64_t iter = iterations;
        __m512i write_buf;
        memset(&write_buf, 0, sizeof(write_buf));
        int *p;
        pthread_barrier_wait(&barrier);
        auto start_timer = std::chrono::system_clock::now();
        while (iterations-- > 0)
        {

            p = ((int *)local_addr);
            while (p < lastone)
            {
                // _mm512_stream_si512((__m512i *)(p + i), write_buf); p[i] = 0;
#define DOIT(i) _mm512_stream_si512((__m512i *)(p + i), write_buf);
#include "op_8_flush.h"
#undef DOIT
                p += 128;
            }
        }
        auto end_timer = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_timer - start_timer);
        std::cout << "[wr time]:[" << duration.count() << "](ms), [wr bw]:[" << 
        working_set_size * iter / duration.count() / 1024 << "](MB/s)" << std::endl;
    };
    

    uint64_t wss = 512 * 1024 * 1024;
    {

        for (int thread = 1; thread <= 20; thread++)
        {
            pthread_barrier_init(&barrier, NULL, thread);
            {
                util::PmmDataCollector measure("dimm");
                std::vector<std::thread> work_group;
                uint64_t iteration = (1ULL << 32) / wss;

                results.resize(thread, 0);
                tid.store(0);
                if ((wss >> 10) % (thread) != 0)
                {
                    continue;
                }
                uint64_t thread_wss = wss / thread;
                pthread_barrier_init(&barrier, NULL, thread);
                for (int i = 0; i < thread; i++)
                {
                    work_group.push_back(std::thread{rd_func, addr + i * thread_wss, thread_wss, iteration});
                }
                for (int i = 0; i < thread; i++)
                {
                    work_group.at(i).join();
                }

                uint64_t time = 0;
                for (auto i : results)
                {
                    time += i;
                }
                std::cout << "-------result--------" << std::endl;
                std::cout << "[thread]:[" << thread << "]" << std::endl;
                std::cout << "[wss]:[" << wss << "]" << std::endl;
                std::cout << "[thread wss]:[" << thread_wss << "]" << std::endl;
                std::cout << "[iteration]:[" << iteration << "]" << std::endl;
                std::cout << "[rd time]:[" << time / thread << "](ms), [rd bw]:[" << thread_wss * iteration * thread / time* thread / 1024 << "](MB/s)" << std::endl;
                std::cout << "---------------------" << std::endl;
                // work_group.push_back(std::thread{wr_func, addr1, wss1, 1ULL << 20});
            }
        }

        // work_group.at(1).join();
    }
}

static __m512i cl_buffer;

#define NPAD 31
struct access_unit_t
{
    struct access_unit_t *next;
    uint64_t pad[NPAD];
};

static inline void wr_clwb(void *addr)
{
    *((int *)addr) += 10;
    _mm_clwb(addr);
    // _mm_sfence();
}

static inline void wr_nt(void *addr)
{
    _mm512_stream_si512((__m512i *)addr, cl_buffer);
    // _mm_sfence();
}

void wr_bw_by_calc(void *addr, uint64_t max_size)
{
    void (*wr_method)(void *);
    memset(&cl_buffer, 0, sizeof(cl_buffer));
    auto func = [&](uint64_t wss, uint64_t iter, bool seq)
    {
        std::cout
            << "-------result--------" << std::endl;
        if (wr_method == wr_nt)
        {
            std::cout << "[type]:[nt store]" << std::endl;
        }
        else if (wr_method == wr_clwb)
        {
            std::cout << "[type]:[clwb]" << std::endl;
        }
        uint64_t node_num = wss / sizeof(access_unit_t);
        std::vector<uint64_t> order(node_num);
        for (uint64_t i = 0; i < node_num; i++)
        {
            order.at(i) = i;
        }
        if (!seq)
        {
            std::cout << "[order]:[rand] ";
            std::random_shuffle(order.begin(), order.end());
        }
        else
        {
            std::cout << "[order]:[seq] ";
        }
        auto work_ptr = (access_unit_t *)addr;
        uint64_t start_tick, end_tick, total_cycles = 0;
        std::chrono::milliseconds duration;
        float imc_read, imc_write, media_read, media_write;
        {
            util::PmmDataCollector measure("DIMM data", &imc_read, &imc_write, &media_read, &media_write);
            auto start = std::chrono::system_clock::now();
            for (int i = 0; i < iter; i++)
            {
                for (uint64_t j = 0; j < node_num; j++)
                {
                    wr_method((void *)(work_ptr + order[j]) + 0);
                    // wr_method((void *)(work_ptr + order[j]) + 64);
                    // wr_method((void *)(work_ptr + order[j]) + 128);
                    // wr_method((void *)(work_ptr + order[j]) + 192);
                    _mm_sfence();
                }
            }
            auto end = std::chrono::system_clock::now();
            duration = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);
        }
        std::cout << "[imc wr]:[" << imc_write
                  << "] [imc rd]:[" << imc_read
                  << "] [media wr]:[" << media_write
                  << "] [media rd]:[" << media_read
                  << "]" << std::endl;
        std::cout << "[wss]:[" << wss << "](Bytes) [bw]: ["
                  << wss * iter / duration.count() * 1000 / 1024 / 1024 
                  << "](MB/sec)" 
                //   << " [time]:["<< duration.count() << "]"
                //   << " [iteration]:[" << iter <<"]"
                  << std::endl;
        std::cout << "---------------------" << std::endl;
    };
    bool seq = false;
    wr_method = wr_nt;
    for (int k = 0; k < 2; k++)
    {
        seq = !seq;
        for (int i = 12; i < 32; i++)
        {
            for (int j = 0; j < 1; j++)
            {
                func((1ULL << i) + j * (1ULL << (i - 2)), (1ULL << (30 - i)), seq);
            }
        }
    }
    wr_method = wr_clwb;
    for (int k = 0; k < 2; k++)
    {
        seq = !seq;
        for (int i = 12; i < 32; i++)
        {
            for (int j = 0; j < 1; j++)
            {
                func((1ULL << i) + j * (1ULL << (i - 2)), (1ULL << (30 - i)), seq);
            }
        }
    }
}


#define GRANULARITY 4096
#define UNROLL_NUM (GRANULARITY / 64)
void wr_bw_in_out_place(void *addr, uint64_t max_size)
{
    void (*wr_method)(void *);
    memset(&cl_buffer, 0, sizeof(cl_buffer));
    auto func = [&](uint64_t wss, uint64_t iter, bool seq)
    {
        std::cout
            << "-------result--------" << std::endl;
        if (wr_method == wr_nt)
        {
            std::cout << "[type]:[nt store]" << std::endl;
        }
        else if (wr_method == wr_clwb)
        {
            std::cout << "[type]:[clwb]" << std::endl;
        }
        std::cout << "[granu]:[" << GRANULARITY << "]" << std::endl;
        uint64_t node_num = wss / sizeof(access_unit_t)/GRANULARITY;
        std::vector<uint64_t> order(node_num);
        for (uint64_t i = 0; i < node_num; i++)
        {
            order.at(i) = i;
        }
        if (!seq)
        {
            std::cout << "[order]:[rand] ";
            std::random_shuffle(order.begin(), order.end());
        }
        else
        {
            std::cout << "[order]:[seq] ";
        }
        auto work_ptr = (access_unit_t *)addr;
        uint64_t start_tick, end_tick, total_cycles = 0;
        std::chrono::milliseconds duration;
        float imc_read, imc_write, media_read, media_write;
        {
            util::PmmDataCollector measure("DIMM data", &imc_read, &imc_write, &media_read, &media_write);
            auto start = std::chrono::system_clock::now();
            for (int i = 0; i < iter; i++)
            {
                for (uint64_t j = 0; j < node_num; j++)
                {
                    #pragma unroll(UNROLL_NUM)
                    for (int k = 0; k < GRANULARITY; k += 64)
                    {
                        #ifndef IN_PLACE
                        wr_method((void *)(work_ptr + order[j]) + k);
                        #else
                        wr_method((void *)(work_ptr + order[0]) + k);
                        #endif
                    }
                    #pragma unroll
                    _mm_sfence();
                }
            }
            auto end = std::chrono::system_clock::now();
            duration = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);
        }
        std::cout << "[imc wr]:[" << imc_write
                  << "] [imc rd]:[" << imc_read
                  << "] [media wr]:[" << media_write
                  << "] [media rd]:[" << media_read
                  << "]" << std::endl;
        std::cout << "[wss]:[" << wss << "](Bytes) [bw]: ["
                  << wss * iter / duration.count() * 1000 / 1024 / 1024 
                  << "](MB/sec)" 
                //   << " [time]:["<< duration.count() << "]"
                //   << " [iteration]:[" << iter <<"]"
                  << std::endl;
        std::cout << "---------------------" << std::endl;
    };
    bool seq = false;
    wr_method = wr_nt;
    for (int k = 0; k < 2; k++)
    {
        seq = !seq;
        for (int i = 12; i < 30; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                func((1ULL << i) + j * (1ULL << (i - 2)), (1ULL << (30 - i)), seq);
            }
        }
    }
    wr_method = wr_clwb;
    for (int k = 0; k < 2; k++)
    {
        seq = !seq;
        for (int i = 12; i < 30; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                func((1ULL << i) + j * (1ULL << (i - 2)), (1ULL << (30 - i)), seq);
            }
        }
    }
}



