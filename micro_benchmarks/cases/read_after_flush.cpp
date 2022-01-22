#include "common.h"
#include <numa.h>

void tmp_test(void *addr, uint64_t max_size);

__m512 data_to_write;

static inline void wr_clwb(void *addr)
{
    *((int *)addr) += 10;
    _mm_clwb(addr);
    // _mm_sfence();
}

static inline void wr_nt(void *addr)
{
    _mm512_stream_ps((float *)addr, data_to_write);
    // _mm512_stream_si512((__m512i *)addr, cl_buffer);
    // _mm_sfence();
}

static inline void rd(void *addr)
{

}


static int user_result_dummy = 0;
void read_after_flush(void *addr, uint64_t max_size)
{
    // posix_memalign(&addr, 64, 1ULL << 12);
    void (*wr_op)(void *);
    auto func = [&](uint64_t working_set_size, int iterations, int cl_distance) {
        char *work_ptr = (char *)addr;
        // volatile int tmp_buf=0;
        float imc_rd, imc_wr, media_rd, media_wr;
        uint64_t start_timer, end_timer;
        register int sum = 0;
        {
            
            memset(&data_to_write, 0, sizeof(data_to_write));
            
            util::PmmDataCollector measure("PMM data", &imc_rd, &imc_wr, &media_rd, &media_wr);
            start_timer = rdtsc();
            for (int i = 0; i < iterations; i++)
            {
                for (uint64_t j = 0; j < working_set_size; j += 64)
                {
                    wr_op(work_ptr + j);
                    // _mm512_stream_ps((float *)(work_ptr + j), data_to_write);
                    //     work_ptr[j] = i;
                    // _mm_clwb(work_ptr + j);
                    _mm_sfence();
                    // for (int k = 0; k < 20; k++)
                    // {
                    //     tmp_buf++;
                    // }
                    // work_ptr[(j + working_set_size - (cl_distance << 6)) % working_set_size] = 0;
                    sum += work_ptr[(j + working_set_size - (cl_distance << 6)) % working_set_size];
                }
            }
            end_timer = rdtsc();
        }
        std::cout << "-------result--------"<< std::endl;
        if(wr_op == wr_nt)
        {
            std::cout << "[method]:[nt]" << std::endl;
        }
        else if (wr_op == wr_clwb)
        {
            std::cout << "[method]:[clwb]" << std::endl;
        }
        std::cout << "[distance]:[" << cl_distance << "],[RAP lat]:["
                  << (end_timer - start_timer) * 1.0 / ((working_set_size >> 6) * iterations)
                  << "] (cycles/cacheline), [WA]: [" << media_wr / imc_wr
                  << "], [RA]: [" << media_rd / imc_rd
                  << "], [imc wr]: [" << imc_wr
                  << "], [imc rd]: [" << imc_rd
                  << "]" << std::endl;
        std::cout << "---------------------"<< std::endl;
        user_result_dummy += sum;
    };
    wr_op = wr_nt;
    for (int i = 0; i < 70; i++)
    {
        func(1ULL << 13, 50000, i);
    }
    wr_op = wr_clwb;
    for (int i = 0; i < 70; i++)
    {
        func(1ULL << 13, 50000, i);
    }
    // free(addr);
}

// this runs on two seperated cores
// void read_after_flush(void *addr, uint64_t max_size)
// {
//     memset(addr, -1, max_size);
//     uint64_t curr_offset = -1;
//     pthread_barrier_t barrier;
//     pthread_barrier_init(&barrier, NULL, 2);
//     std::atomic<int> variable;
//     std::cout<< "size is " << sizeof(variable) << std::endl;
//     auto wr_func = [&](uint64_t working_set_size, int iterations) {
//         std::vector<uint64_t> read_sequence((working_set_size >> 6) * iterations, -1);
//         int counter = 0;
//         register int sum=0;
//         uint64_t *work_ptr = (uint64_t *)addr;
//         // volatile int tmp_buf=0;
//         pthread_barrier_wait(&barrier);
//         auto start_ticker = rdtsc();
//         for (int i = 0; i < iterations; i++)
//         {
//             for (uint64_t j = 0; j < (working_set_size / 8); j += 8)
//             {
//                 // work_ptr[j] = i;
//                 // _mm_clflush(work_ptr + j);
//                 _mm_stream_si64 ((long long *)(work_ptr + j), (long long) i);
//                 _mm_sfence();
//                 // sum += work_ptr[j];
//                 while (1)
//                 {
//                     if (curr_offset == j)
//                     {
//                         read_sequence.at(counter) = j;
//                         counter++;
//                         break;
//                     }
//                 }
//             }
//         }
//         user_result_dummy += sum;
//         auto end_ticker = rdtsc();
//         std::cout << "time is " << (end_ticker - start_ticker) / (working_set_size / 64) / iterations << std::endl;
//     };

//     auto rd_func = [&](uint64_t working_set_size, int iterations) {
//         std::vector<uint64_t> read_sequence((working_set_size >> 6) * iterations, -1);
//         int counter = 0;
//         uint64_t *work_ptr = (uint64_t *)addr;
//         // volatile int tmp_buf=0;
//         pthread_barrier_wait(&barrier);
//         std::atomic<uint64_t> *atomic_ptr = (std::atomic<uint64_t> *)work_ptr;
//         for (int i = 0; i < iterations; i++)
//         {
//             for (uint64_t j = 0; j < (working_set_size / 8); j += 8)
//             {
//                 while (1)
//                 {
//                     if (atomic_ptr[j].load() == i)
//                     {
//                         read_sequence.at(counter) = i;
//                         curr_offset = j;
//                         counter++;
//                         break;
//                     }
//                 }
//             }
//         }
//     };
//     std::vector<std::thread> work_group;
//     {
//         util::PmmDataCollector measure("dimm");
//         work_group.push_back(std::thread{rd_func, 4096, 1ULL << 20});
//         work_group.push_back(std::thread{wr_func, 4096, 1ULL << 20});
//         work_group.at(0).join();
//         work_group.at(1).join();
//     }
// }

// #define ON_DRAM
void read_after_flush_lock_contention(void *addr, uint64_t max_size)
{
    int thread_lock_on_dram;
    std::atomic<int> *thread_lock = (std::atomic<int> *)&thread_lock_on_dram;
    uint64_t start_tick, end_tick, across_thread_lat = 0;
#ifdef ON_DRAM
    posix_memalign((void **)&thread_lock, 64, 64);
#endif
    // pthread_spinlock_t *thread_lock = (pthread_spinlock_t *)addr;
    *thread_lock = 0;
    pthread_barrier_t barrier;
    volatile uint64_t total_hold_lock_num = 0;
    volatile uint64_t T1_miss = 0;
    volatile uint64_t T2_miss = 0;
    volatile uint64_t T1_lat = 0;
    volatile uint64_t T2_lat = 0;
    pthread_barrier_init(&barrier, NULL, 2);
    int end_flag = 0;
    auto T1 = [&]() {
        register uint64_t count = 0, miss = 0, spin_round = 0, latency = 0;
        int target_unlock = 0, target = 1, self_unlock = 2;
        pthread_barrier_wait(&barrier);
        char *work_ptr = (char *)addr;
        while (1)
        {
            if (end_flag == 1)
                break;
            // try to get lock
            auto start = rdtsc();
            if (thread_lock->compare_exchange_weak(target_unlock, target))
            {
                count++;
                start_tick = rdtsc();
                _mm_mfence();
                work_ptr[0] = 10;
                _mm_clwb(work_ptr);
                // reset lock status
                // _mm_stream_si32((int *)thread_lock, self_unlock);
                *thread_lock = self_unlock;
                // _mm_clwb(thread_lock);
                _mm_sfence();
            }
            auto end = rdtsc();
            if (target_unlock != 0)
                miss++;
            target_unlock = 0;
            latency += end - start;
            spin_round += 1;
        }
        total_hold_lock_num = count;
        T1_miss = miss;
        T1_lat = latency / spin_round;
    };
    auto T2 = [&]()
    {
        register uint64_t miss = 0, latency = 0, spin_round = 0, sum = 0;
        cpu_set_t local_cpu;
        CPU_ZERO(&local_cpu);
        CPU_SET(9, &local_cpu);
        char *work_ptr = (char *)addr;
        pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &local_cpu);

        pthread_barrier_wait(&barrier);
        int target_unlock = 2, target = 3, self_unlock = 0;
        while (1)
        {
            if (end_flag == 1)
                break;
            auto start = rdtsc();
            if (thread_lock->compare_exchange_weak(target_unlock, target))
            {
                // _mm_stream_si32((int *)thread_lock, self_unlock);
                sum += work_ptr[0];
                _mm_mfence();
                end_tick = rdtsc();
                across_thread_lat += end_tick - start_tick;
                *thread_lock = self_unlock;
                // _mm_clwb(thread_lock);
                _mm_sfence();
            }
            auto end = rdtsc();
            latency += end -start;
            if (target_unlock != 2)
                miss++;
            target_unlock = 2;
            spin_round += 1;
        }
        user_result_dummy += sum;
        T2_miss = miss;
        T2_lat = latency / spin_round;
    };

    // tmp_test(addr, 100);

    auto start = rdtsc();
    std::thread instance0(T1);

    std::thread instance1(T2);
    std::this_thread::sleep_for(std::chrono::seconds(30));
    end_flag = 1;
    instance0.join();
    instance1.join();
    auto end = rdtsc();
    std::cout << "T1 get lock for: " << total_hold_lock_num << std::endl;
    std::cout << "T1 miss for: " << T1_miss << std::endl;
    std::cout << "T2 miss for: " << T2_miss << std::endl;
    std::cout << "T1 latency is: " << T1_lat << std::endl;
    std::cout << "T2 latency is: " << T2_lat << std::endl;
    std::cout << (end - start) / total_hold_lock_num << " cycles/round" << std::endl;
    std::cout << "across thread latency is: " << across_thread_lat / total_hold_lock_num << std::endl;
#ifdef ON_DRAM
    free(thread_lock);
#endif
#undef ON_DRAM
}

void tmp_test(void *addr, uint64_t max_size)
{
    char *ptr = (char *)addr;
    register int tmp = 0;
    int iter = (1ULL << 17);
    int wss = 192;
    util::PmmDataCollector measure("dimm");
    auto start = rdtsc();
    for (uint64_t i = 0; i < iter; i++)
    {
        for (int j = 0; j < wss; j += 64)
        {
            ptr[j] = 0;
            _mm_clflush(ptr + j);
            _mm_mfence();
            tmp += ptr[j];
        }
    }
    user_result_dummy += tmp;
    auto end = rdtsc();
    std::cout << (end - start) / iter / (wss/64)<< " cycles/round" << std::endl;
}