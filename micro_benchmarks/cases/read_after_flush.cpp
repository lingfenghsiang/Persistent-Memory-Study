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

static inline void wr_clwb_sfence(void *addr)
{
    *((int *)addr) += 10;
    _mm_clwb(addr);
    _mm_sfence();
}

static inline void wr_clwb_mfence(void *addr)
{
    *((int *)addr) += 10;
    _mm_clwb(addr);
    _mm_mfence();
}

static inline void wr_nt(void *addr)
{
    _mm512_stream_ps((float *)addr, data_to_write);
    // _mm512_stream_si512((__m512i *)addr, cl_buffer);
    // _mm_sfence();
}

static inline void wr_nt_sfence(void *addr)
{
    _mm512_stream_ps((float *)addr, data_to_write);
    // _mm512_stream_si512((__m512i *)addr, cl_buffer);
    _mm_sfence();
}

static inline void wr_nt_mfence(void *addr)
{
    _mm512_stream_ps((float *)addr, data_to_write);
    // _mm512_stream_si512((__m512i *)addr, cl_buffer);
    _mm_mfence();
}

static inline void rd(void *addr)
{
}

static int user_result_dummy = 0;
void read_after_flush(void *addr, uint64_t max_size)
{
    // posix_memalign(&addr, 64, 1ULL << 12);
    void (*wr_op)(void *);
    auto func = [&](uint64_t working_set_size, int iterations, int cl_distance)
    {
        char *work_ptr = (char *)addr;
        // volatile int tmp_buf=0;
        float imc_rd = 0, imc_wr = 0, media_rd = 0, media_wr = 0;
        uint64_t start_timer, end_timer;
        register int sum = 0;
        {

            memset(&data_to_write, 0, sizeof(data_to_write));

            // util::PmmDataCollector measure("PMM data", &imc_rd, &imc_wr, &media_rd, &media_wr);
            start_timer = rdtsc();
            for (int i = 0; i < iterations; i++)
            {
                for (uint64_t j = 0; j < working_set_size; j += 64)
                {
                    *((int *)(work_ptr + j)) += 10;
                    _mm_clwb(work_ptr + j);
                    // _mm_mfence();
                    //                 wr_op(work_ptr + j);
                    //                 // _mm512_stream_ps((float *)(work_ptr + j), data_to_write);
                    //                 //     work_ptr[j] = i;
                    //                 // _mm_clwb(work_ptr + j);
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
        std::cout << "-------result--------" << std::endl;
        if (wr_op == wr_nt)
        {
            std::cout << "[method]:[nt]" << std::endl;
        }
        else if (wr_op == wr_clwb)
        {
            std::cout << "[method]:[clwb]" << std::endl;
        }
        else if (wr_op == wr_clwb_sfence)
        {
            std::cout << "[method]:[wr_clwb_sfence]" << std::endl;
        }
        else if (wr_op == wr_clwb_mfence)
        {
            std::cout << "[method]:[wr_clwb_mfence]" << std::endl;
        }
        else if (wr_op == wr_nt_sfence)
        {
            std::cout << "[method]:[wr_nt_sfence]" << std::endl;
        }
        else if (wr_op == wr_nt_mfence)
        {
            std::cout << "[method]:[wr_nt_mfence]" << std::endl;
        }
        std::cout << "[distance]:[" << cl_distance << "],[RAP lat]:["
                  << (end_timer - start_timer) * 1.0 / ((working_set_size >> 6) * iterations)
                  << "] (cycles/cacheline), [WA]: [" << media_wr / imc_wr
                  << "], [RA]: [" << media_rd / imc_rd
                  << "], [imc wr]: [" << imc_wr
                  << "], [imc rd]: [" << imc_rd
                  << "]" << std::endl;
        std::cout << "---------------------" << std::endl;
        user_result_dummy += sum;
    };
    // wr_op = wr_nt_mfence;
    // for (int i = 0; i < 70; i++)
    // {
    //     func(1ULL << 13, 50000, i);
    // }
    // wr_op = wr_clwb_mfence;
    // for (int i = 0; i < 70; i++)
    // {
    //     func(1ULL << 13, 50000, i);
    // }
    // wr_op = wr_nt_sfence;
    // for (int i = 0; i < 70; i++)
    // {
    //     func(1ULL << 13, 50000, i);
    // }
    // wr_op = wr_clwb_sfence;
    // for (int i = 0; i < 70; i++)
    // {
    //     func(1ULL << 13, 50000, i);
    // }
    wr_op = wr_clwb_sfence;
    util::debug_perf_ppid();
    util::debug_perf_switch();
    func(1ULL << 13, 5000000, 0);
    util::debug_perf_switch();
}

void read_after_flush_lock_contention(void *addr, uint64_t max_size)
{

    int thread_lock_on_dram;
    std::atomic<int> *thread_lock = (std::atomic<int> *)&thread_lock_on_dram;
    uint64_t start_tick, end_tick, across_thread_lat = 0;
    // pthread_spinlock_t *thread_lock = (pthread_spinlock_t *)addr;
    *thread_lock;
    pthread_barrier_t barrier;
    volatile uint64_t total_hold_lock_num;
    volatile uint64_t T1_miss;
    volatile uint64_t T2_miss;
    volatile uint64_t T1_lat;
    volatile uint64_t T2_lat;
    int end_flag = 0;

    if (numa_available() < 0)
    {
        perror("numa not available");
        exit(EXIT_FAILURE);
    }

    struct bitmask *cpu_mask = numa_allocate_cpumask();
    cpu_set_t node_cpu[2];
    for (int i = 0; i < 2; i++)
    {
        CPU_ZERO(node_cpu + i);
        if (numa_node_to_cpus(i, cpu_mask))
        {
            perror("numa_node");
            exit(EXIT_FAILURE);
        };

        for (int j = 0; j < cpu_mask->size; j++)
        {
            unsigned long idx = j / (sizeof(*cpu_mask->maskp) * 8);
            unsigned long bit = j % (sizeof(*cpu_mask->maskp) * 8);
            if (cpu_mask->maskp[idx] & (1UL << j))
            {
                CPU_SET(j, node_cpu + i);
            }
        }
    }

    numa_free_cpumask(cpu_mask);

    auto T1 = [&](uint64_t wss)
    {
        pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), node_cpu + 0);
        register uint64_t count = 0, miss = 0, spin_round = 0, latency = 0;
        int target_unlock = 0, target = 1, self_unlock = 2, j = 0;
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
                work_ptr[j] = 10;
                _mm_clwb(work_ptr + j);
                j += 64;
                if (j > wss)
                {
                    j = 0;
                }
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
    auto T2 = [&](uint64_t wss, int distance, bool read_flag)
    {
        pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), node_cpu + 1);
        register uint64_t miss = 0, latency = 0, spin_round = 0, sum = 0;
        char *work_ptr = (char *)addr;

        pthread_barrier_wait(&barrier);
        int target_unlock = 2, target = 3, self_unlock = 0, j = 0;
        while (1)
        {
            if (end_flag == 1)
                break;
            auto start = rdtsc();
            if (thread_lock->compare_exchange_weak(target_unlock, target))
            {
                // _mm_stream_si32((int *)thread_lock, self_unlock);
                if (read_flag)
                    sum += work_ptr[(j + wss - (distance << 6)) % wss];
                _mm_mfence();
                end_tick = rdtsc();
                j += 64;
                if (j > wss)
                {
                    j = 0;
                }
                across_thread_lat += end_tick - start_tick;
                *thread_lock = self_unlock;
                // _mm_clwb(thread_lock);
                _mm_sfence();
            }
            auto end = rdtsc();
            latency += end - start;
            if (target_unlock != 2)
                miss++;
            target_unlock = 2;
            spin_round += 1;
        }
        user_result_dummy += sum;
        T2_miss = miss;
        T2_lat = latency / spin_round;
    };

    auto run = [&](uint64_t wss, int distance, bool t2_read)
    {
        pthread_barrier_init(&barrier, NULL, 2);
        across_thread_lat = 0;
        *thread_lock = 0;
        total_hold_lock_num = 0;
        T1_miss = 0;
        T2_miss = 0;
        T1_lat = 0;
        T2_lat = 0;
        end_flag = 0;
        auto start = rdtsc();
        std::thread instance0(T1, wss);
        std::thread instance1(T2, wss, distance, t2_read);
        std::this_thread::sleep_for(std::chrono::seconds(30));
        end_flag = 1;
        instance0.join();
        instance1.join();
        if (numa_run_on_node(-1) < 0)
        {
            perror("numa");
            exit(EXIT_FAILURE);
        };

        auto end = rdtsc();
        std::cout << "-------result--------" << std::endl;
        std::cout << "[T1 get lock]:[" << total_hold_lock_num << "]" << std::endl;
        std::cout << "[T1 miss]:[" << T1_miss << "]" << std::endl;
        std::cout << "[T2 miss]:[" << T2_miss << "]" << std::endl;
        std::cout << "[T1 latency]:[" << T1_lat << "]" << std::endl;
        std::cout << "[T2 latency]:[" << T2_lat << "]" << std::endl;
        std::cout << "[distance]:[" << distance << "]" << std::endl;
        std::cout << "[Latency per round]:[" << (end - start) / total_hold_lock_num << "](cycles/round)" << std::endl;
        std::cout << "[across thread latency]:[" << across_thread_lat / total_hold_lock_num << "]" << std::endl;
        if (t2_read)
            std::cout << "[T2 read]:[" << 1 << "]" << std::endl;
        else
            std::cout << "[T2 read]:[" << 0 << "]" << std::endl;
        std::cout << "---------------------" << std::endl;
    };
    for (int i = 0; i < 20; i++)
    {
        run(50000, i, true);
    }
    for (int i = 0; i < 20; i++)
    {
        run(50000, i, false);
    }
}
