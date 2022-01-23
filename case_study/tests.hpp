#pragma once
#include "global.hpp"
#include <x86intrin.h>
#include <numeric>
#include <string.h>
#include "kv_wrap.hpp"
#include "pm_util.h"

enum operation
{
    READ,
    INSERT,
    DELETE,
    UPDATE,
    READ_NOTFOUND
};

inline uint64_t rdtsc()
{
    unsigned int lo, hi;
    __asm__ __volatile__("rdtsc"
                         : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

// cacheline aligned
struct sync_buffer
{
    uint64_t curr_progress;
    uint64_t begin;
    uint64_t end;
    char pad[40];
};

template <typename T>
float average(std::vector<T> &array, uint64_t begin, uint64_t end, int stride)
{
    float result = 0;

    if (end - begin < stride)
    {
        for (uint64_t i = begin; i < end; i++)
        {
            result += array.at(i);
        }
        result /= (end - begin);
    }
    else
    {
        result = ((((begin + end) / 2) - begin) * 1.0 / (end - begin) *
                      average(array, begin, (begin + end) / 2, stride) +
                  (end - (begin + end) / 2) * 1.0 /
                      (end - begin) *
                      average(array, (begin + end) / 2, end, stride));
    }
    return result;
}

template <typename T1, typename T2>
void test(Index<T1, T2> &index, std::vector<std::pair<T1, T2>> &run_pairs, std::vector<operation> &runtime_ops,
                                       int thread_num, bool helper_mod, int prereadnum, std::string notion)
{
    int keys_num = run_pairs.size();
    std::cout << notion << ":" << std::endl;
    std::atomic<uint64_t> finished_num;
    std::vector<uint64_t> latency(keys_num, 0);
    sync_buffer *shared_sync_buffer = NULL;
    volatile int end_flag = 0;
    std::vector<std::thread> preread_group;
    std::vector<std::atomic<uint64_t>> global_status(5);

    for (int i = 0; i < 5; i++)
    {
        global_status.at(i).store(0);
    }

    // set up preread threads
    if (helper_mod)
    {

        if (!posix_memalign((void **)&shared_sync_buffer, 64, 64 * thread_num))
        {
            memset(shared_sync_buffer, 0, 64 * thread_num);
        }
        else
        {
            return;
        };
        std::atomic<int> thread_id(0);
        int preread_shreshold = prereadnum;

        auto preread = [&]()
        {
            int t_id = thread_id.fetch_add(1);
            if (t_id + 1 > thread_num)
                return;
            sync_buffer *local_sync_buf = shared_sync_buffer + t_id;

            sync_buffer local_copy;
            uint64_t progerss;
            while (1)
            {
                local_copy = *local_sync_buf;
                // tmp_count +=(i);
                if (end_flag == 1)
                    return;
                else if ((progerss - local_copy.curr_progress) > preread_shreshold)
                {
                    progerss = local_copy.curr_progress + preread_shreshold;
                    _mm_pause();
                    _mm_pause();
                    _mm_pause();
                }
                else if (progerss >= keys_num)
                {
                    _mm_pause();
                    _mm_pause();
                    _mm_pause();
                }
                else if ((progerss - local_copy.curr_progress) < 0)
                {
                    progerss = local_copy.curr_progress + 1;
                }
                else
                {
                    // if (runtime_ops.at(progerss) == operation::INSERT)
                    index.Touch(run_pairs.at(progerss).first);
                    progerss++;
                }
            }
        };
        
        preread_group.reserve(thread_num);
        for (int i = 0; i < thread_num; i++)
        {
            preread_group.push_back(std::thread(preread));
        }
    }

    std::vector<std::thread> works;
    std::atomic<uint64_t> progress(0);

    auto worker = [&index, &progress, &run_pairs, &runtime_ops, &latency, &global_status](uint64_t mini_batch)
    {
        uint64_t max_ops = run_pairs.size(), start_clock, end_clock;
        uint64_t start_idx = progress.fetch_add(mini_batch), end_idx = std::min(max_ops, start_idx + mini_batch);
        uint64_t status[5];
        status[operation::READ] = 0;
        status[operation::INSERT] = 0;
        status[operation::UPDATE] = 0;
        status[operation::DELETE] = 0;
        status[operation::READ_NOTFOUND] = 0;
        while (start_idx < max_ops)
        {
            for (int i = start_idx; i < end_idx; i++)
            {
                start_clock = rdtsc();
                if (runtime_ops.at(i) == operation::READ)
                {
                    status[operation::READ]++;
                    T2 rd = index.Get(run_pairs.at(i).first);
                    if (rd != run_pairs.at(i).second)
                        status[operation::READ_NOTFOUND]++;
                }
                else if (runtime_ops.at(i) == operation::INSERT)
                {
                    status[operation::INSERT]++;
                    int result = index.Insert(run_pairs.at(i).first, run_pairs.at(i).second);
                }
                else if (runtime_ops.at(i) == operation::UPDATE)
                {
                    status[operation::UPDATE]++;
                    index.Update(run_pairs.at(i).first, run_pairs.at(i).second);
                }
                else if (runtime_ops.at(i) == operation::DELETE)
                {
                    status[operation::DELETE]++;
                    index.Delete(run_pairs.at(i).first);
                }
                end_clock = rdtsc();
                latency.at(i) = end_clock - start_clock;
            }
            start_idx = progress.fetch_add(mini_batch);

            end_idx = std::min(max_ops, start_idx + mini_batch);
        }
        for (int i = 0; i < 5; i++)
        {
            global_status.at(i).fetch_add(status[i]);
        }
    };

    // execute phase
    util::ProgressShow progerss(&progress, keys_num);
    auto start_timer = std::chrono::system_clock::now();
    float imc_rd, imc_wr, media_wr, media_rd;
    {
        util::PmmDataCollector measure("pmm data", &imc_rd, &imc_wr, &media_rd, &media_wr);
        measure.DisablePrint();
        for (int i = 0; i < thread_num; i++)
        {
            works.push_back(std::thread{worker, 10000});
        }
        for (int i = 0; i < thread_num; i++)
        {
            works.at(i).join();
        }
    }
    auto end_timer = std::chrono::system_clock::now();
    // tbb::parallel_for(tbb::blocked_range<int>(0, keys_num), [&](tbb::blocked_range<int> &tbb_range) {
    //     uint64_t start_clock, end_clock;
    //     int worker_id = tbb::task_arena::current_thread_index();
    //     // auto local_sync_buf = shared_sync_buffer + worker_id;
    //     // local_sync_buf->curr_progress = local_sync_buf->begin = tbb_range.begin();
    //     // local_sync_buf->end = tbb_range.end();
    //     uint64_t status[5];
    //     status[operation::READ] = 0;
    //     status[operation::INSERT] = 0;
    //     status[operation::UPDATE] = 0;
    //     status[operation::DELETE] = 0;
    //     status[operation::READ_NOTFOUND] = 0;

    //     for (int i = tbb_range.begin(); i < tbb_range.end(); i++)
    //     {
    //         tmp_idx++;
    //         if (tmp_idx % 1000 == 0)
    //         {
    //             std::cout << tmp_idx << std::endl;
    //         }
    //         index.Insert(run_pairs[i].first, run_pairs[i].second);
    //     }

    //         //     start_clock = rdtsc();
    //         //     if (runtime_ops.at(i) == operation::READ)
    //         //     {
    //         //         status[operation::READ]++;
    //         //         T2 r = index.Get(run_pairs.at(i).first);
    //         //         if (r != run_pairs.at(i).second)
    //         //             status[operation::READ_NOTFOUND]++;
    //         //     }
    //         //     else if (runtime_ops.at(i) == operation::INSERT)
    //         //     {
    //         //         status[operation::INSERT]++;
    //         //         int result = index.Insert(run_pairs.at(i).first, run_pairs.at(i).second);
    //         //     }
    //         //     else if (runtime_ops.at(i) == operation::UPDATE)
    //         //     {
    //         //         status[operation::UPDATE]++;
    //         //         index.Update(run_pairs.at(i).first, run_pairs.at(i).second);
    //         //     }
    //         //     else if (runtime_ops.at(i) == operation::DELETE)
    //         //     {
    //         //         status[operation::DELETE]++;
    //         //         index.Delete(run_pairs.at(i).first);
    //         //     }
    //         //     end_clock = rdtsc();
    //         //     latency.at(i) = end_clock - start_clock;
    //         //     finished_num.fetch_add(1);
    //         //     local_sync_buf->curr_progress++;
    //         // }
    //         // for (int i = 0; i < 5; i++)
    //         // {
    //         //     global_status.at(i).fetch_add(status[i]);
    //         // }
    //     });
    //     auto func = [&]()
    //     {
    //         uint64_t start_clock, end_clock;
    //         int worker_id = tbb::task_arena::current_thread_index();
    //         auto local_sync_buf = shared_sync_buffer + worker_id;
    //         local_sync_buf->curr_progress = local_sync_buf->begin = 0;
    //         local_sync_buf->end = keys_num;
    //         uint64_t status[5];
    //         status[operation::READ] = 0;
    //         status[operation::INSERT] = 0;
    //         status[operation::UPDATE] = 0;
    //         status[operation::DELETE] = 0;
    //         status[operation::READ_NOTFOUND] = 0;

    //         for (int i = 0; i < keys_num; i++)
    //         {
    //             start_clock = rdtsc();
    //              if (runtime_ops.at(i) == operation::INSERT)
    //              {
    //                  status[operation::INSERT]++;
    //                  int result = index.Insert(run_pairs.at(i).first, run_pairs.at(i).second);
    //              }

    //              end_clock = rdtsc();
    //              latency.at(i) = end_clock - start_clock;
    //              finished_num.fetch_add(1);
    //              local_sync_buf->curr_progress++;
    //         }
    //         for (int i = 0; i < 5; i++)
    //         {
    //             global_status.at(i).fetch_add(status[i]);
    //         }
    //     };
    //     func();
    // };

    // // total_end_clock = rdtsc();
    
    if (helper_mod)
    {
        end_flag = 1;
        for (int i = 0; i < thread_num; i++)
        {
            preread_group.at(i).join();
        }
        free(shared_sync_buffer);
    }
    std::sort(latency.begin(), latency.end());
    uint64_t avg_clock = average(latency, 0, keys_num, 10000);
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_timer - start_timer);
    std::cout << "-------result--------" << std::endl;
    std::cout << "[type]:[" << notion << "]" << std::endl;
    std::cout << "[helper]:[" << helper_mod << "]" << std::endl;
    std::cout << "[thread]:[" << thread_num << "]" << std::endl;
    std::cout << "[Insert]: "
              << "[" << global_status.at(operation::INSERT).load() << "]" << std::endl;
    std::cout << "[Get]: [" << global_status.at(operation::READ).load() << "]" << std::endl;
    std::cout << "[Not found]: [" << global_status.at(operation::READ_NOTFOUND).load() << "]" << std::endl;
    std::cout << "[Update]: [" << global_status.at(operation::UPDATE).load() << "]" << std::endl;
    std::cout << "[Delete]: [" << global_status.at(operation::DELETE).load() << "]" << std::endl;
    std::cout << "[Throughput]:[" << keys_num / 1000.0 / duration.count() << "]"
              << " (Mops/s)" << std::endl;
    std::cout << "[Avg latency]:[" << avg_clock << "]"
              << " (CPU clocks)" << std::endl;
    std::cout << "[90% tail latency]:[" << latency.at((uint64_t)(keys_num * 0.9)) << "]"
              << " (CPU clocks)" << std::endl;
    std::cout << "[99% tail latency]:[" << latency.at((uint64_t)(keys_num * 0.99)) << "]"
              << " (CPU clocks)" << std::endl;
    std::cout << "[iMC read]: [" << imc_rd << "]"
              << "(MB), [imc write]: ["
              << imc_wr << "](MB), [media read]: ["
              << media_rd << "]"
              << "(MB), [media write]: [" << media_wr << "](MB)" << std::endl;
    std::cout << "[RA]: [" << media_rd / imc_rd << "] [WA]: [" << media_wr / imc_wr << "]" << std::endl;
    std::cout << "---------------------" << std::endl;
}