#include <algorithm>
#include "./common.h"
#include "xlf_util.h"
#include <gflags/gflags.h>

VMEM *vmp;
cpu_set_t native_cpuset[HYPER_THERAD_CORE_NUM/SOCKET_NUM];
// access pattern
enum
{
    SEQ,
    RAND
};
motherboard_cpu_layout *cpu_info_ptr = (motherboard_cpu_layout *)cpus_on_board;



DEFINE_string(wss, "1G", "working set size");
DEFINE_uint64(iter, 10, "num of iteration");
DEFINE_int32(thread, 1, "thread number");
DEFINE_bool(seq, true, "if run this in sequential order");
DEFINE_int32(gran, 512, "granularity to run benchmark");

int main(int argc, char **argv)
{
    // default parameters
    uint64_t working_set_size;
    uint64_t iteration;
    int thread_num;
    int order = SEQ;
    int granularity;
    // --------------------------------------------------------------------------
    std::cout << "---------------------Test start-----------------------" << std::endl;
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    iteration = FLAGS_iter;
    thread_num = FLAGS_thread;
    granularity = FLAGS_gran;
    if (FLAGS_seq)
    {
        order = SEQ;
    }
    else
    {
        order = RAND;
    }
    {
        int str_len = FLAGS_wss.length();
        for (int i = 0; i < str_len - 2; i++)
        {
            if (std::isdigit(FLAGS_wss.at(i)))
                continue;
            else
            {
                perror("illegal size");
                return -1;
            }
        }
        auto base_size = FLAGS_wss.substr(0, str_len - 1);
        if (FLAGS_wss.at(str_len - 1) == 'B')
        {
            working_set_size = std::stoull(base_size);
        }
        else if (FLAGS_wss.at(str_len - 1) == 'K')
        {
            working_set_size = std::stoull(base_size) * (1ULL << 10);
        }
        else if (FLAGS_wss.at(str_len - 1) == 'M')
        {
            working_set_size = std::stoull(base_size) * (1ULL << 20);
        }
        else if (FLAGS_wss.at(str_len - 1) == 'G')
        {
            working_set_size = std::stoull(base_size) * (1ULL << 30);
        }
        else
        {
            perror("illegal size");
            return -1;
        }
    }

    
    std::cout << "working set size: " << working_set_size << " Bytes"<< std::endl
    <<"thread number: " << thread_num << std::endl
    <<"granularity: " << granularity << " Bytes" << std::endl
    << "iteration: " << iteration << std::endl;
    if (order == SEQ)
    {
        std::cout << "sequential order" << std::endl;
    }
    else if (order == RAND)
    {
        std::cout << "random order" << std::endl;
    };

    // bind cores

    for (int j = 0; j < HYPER_THERAD_CORE_NUM / PHY_CORE_NUM; j++)
    {
        for (int i = 0; i < PHY_CORE_NUM/SOCKET_NUM; i++)
        {
            CPU_ZERO(native_cpuset + j * PHY_CORE_NUM/SOCKET_NUM + i);
            CPU_SET(cpu_info_ptr->cpu_socket_[0].cpu_physical_[i].hyperthread_core_[j],
                    native_cpuset + j * PHY_CORE_NUM/SOCKET_NUM + i);
        }
    }

    cpu_set_t local_cpu;
    CPU_ZERO(&local_cpu);
    CPU_SET(cpu_info_ptr->cpu_socket_[0].cpu_physical_[0].hyperthread_core_[0], &local_cpu);

    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &local_cpu);

    // prepare pmm pool
    if ((vmp = vmem_create(VOLATILE_POOL_DIR,
                           POOL_SIZE)) == NULL)
    {
        perror("vmem_create");
        exit(1);
    }

    // prepare memory
    void *test_addr_namespace;
    if ((test_addr_namespace = vmem_aligned_alloc(vmp, 256, working_set_size * thread_num)) == NULL)
    {
        perror("vmem_malloc");
        exit(1);
    }
    memset(test_addr_namespace, 0, (working_set_size * thread_num));

    {
        // prepare barrier
        pthread_barrier_t barrier;
        pthread_barrier_init(&barrier, NULL, thread_num);
        // run
        std::atomic<int> thread_index;
        thread_index.store(0);

        // to-run code
        void (*funcTotest[15])(void *, uint64_t, int, std::vector<uint32_t>*);
        funcTotest[0] = frd_64B;
        funcTotest[1] = frd_128B;
        funcTotest[2] = frd_256B;
        funcTotest[3] = frd_512B;
        funcTotest[4] = fwr_512B;
        // funcTotest[5] = frd_64B;
        // funcTotest[6] = frd_64B;
        // funcTotest[7] = frd_64B;
        // funcTotest[8] = frd_64B;
        // funcTotest[9] = frd_64B;
     

        std::vector<uint32_t> access_order(working_set_size/granularity);
            for (uint64_t i = 0; i < working_set_size / granularity; i++)
            {
                access_order.at(i) = i;
            }
            if(order==RAND){
                std::random_shuffle(access_order.begin(), access_order.end());
            }

        auto func = [&]() {
            int thread_id = thread_index.fetch_add(1);
            
            

            pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &native_cpuset[thread_id]);
            void *addr = test_addr_namespace + working_set_size * thread_id;
            pthread_barrier_wait(&barrier);
            funcTotest[4](addr, working_set_size, iteration, &access_order);
        };
        std::vector<std::thread> thread_group;
        
        auto start = std::chrono::system_clock::now();
        float imc_rd, imc_wr, media_rd, media_wr;
        {
            util::PmmDataCollector measure("per dimm data", &imc_rd, &imc_wr, &media_rd, &media_wr);
            for (int i = 0; i < thread_num; i++)
            {
                thread_group.push_back(std::thread{func});
            }
            for (int i = 0; i < thread_num; i++)
            {
                thread_group.at(i).join();
            }
        }
        auto end = std::chrono::system_clock::now();

        pthread_barrier_destroy(&barrier);
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::cout << "Throughput is " << working_set_size * iteration * 1.0 / duration.count() * thread_num << "MB/s" << std::endl;
        std::cout << "Media read throughput is " << media_rd * 1000000.0 / duration.count()  << "MB/s" << std::endl;
        std::cout << "Time is " << duration.count()/1000000.0 << "sec" << std::endl;
    }
    vmem_free(vmp, test_addr_namespace);
    vmem_delete(vmp);
    std::cout << "---------------------Test over-----------------------" << std::endl;
    return 0;
}