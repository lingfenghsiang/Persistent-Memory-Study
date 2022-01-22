#include "common.h"

#define NPAD 31
#define ROUND_UP(x, y) (((x) + (y - 1)) & ~(y - 1))
#define ROUND_DOWN(x, y) ((x) & ~(y - 1))
struct access_unit_t
{
    struct access_unit_t *next;
    uint64_t pad[NPAD];
};

enum TraverseType
{
    CHASING_PTR,
    CALC_OFFSET
};

static __m512i cl_buffer;

static inline void wr_clwb(void *addr)
{
    *((int *)addr) += 10;
    _mm_clwb(addr);
    _mm_sfence();
}

static inline void wr_nt(void *addr)
{
    _mm512_stream_si512((__m512i *)addr, cl_buffer);
    _mm_sfence();
}

static inline void rd(void *addr)
{
    _mm_mfence();
}

struct FeedBackUnit
{
    std::string work_type_;
    std::string order_;
    std::string traverse_type_;
    uint64_t wss_, avg_cycles_;
};

void access_lat(void *addr, uint64_t max_size)
{
    void (*wr_method)(void *);
    memset(&cl_buffer, 0, sizeof(cl_buffer));
    pthread_barrier_t sync_barrier;
    float imc_read, imc_write, media_read, media_write;
    util::PmmDataCollector *measure;
    struct
    {
        float imc_read, imc_write, media_read, media_write;
        std::atomic<int> lock;
    } result;

    auto worker = [&](void *base_addr, uint64_t wss, uint64_t iter, bool seq, TraverseType trav_tp,FeedBackUnit *ret)
    {
        access_unit_t *work_ptr = (access_unit_t *)base_addr;
        auto unit_num = wss / sizeof(access_unit_t);
        std::vector<uint64_t> traverse_order(unit_num);
        int unlock_stat = 0, lock_stat = 1;
        uint64_t total_cycles = 0;
        {
            // init info
            if (max_size <= wss)
            {
                perror("too large wss");
                return;
            }

            if(trav_tp == CHASING_PTR){
                ret->traverse_type_ = "[Trav]:[Ptr]";
            }
            else if (trav_tp == CALC_OFFSET)
            {
                ret->traverse_type_ = "[Trav]:[Calc]";
            }

            if (wr_method == rd)
            {
                ret->work_type_ = "[type]:[rd]";
            }
            else if (wr_method == wr_nt)
            {
                ret->work_type_ = "[type]:[nt store]";
            }
            else if (wr_method == wr_clwb)
            {
                ret->work_type_ = "[type]:[clwb]";
            }
        }
        {
            // prepare order
            for (int i = 0; i < unit_num; i++)
            {
                traverse_order.at(i) = i;
            }

            if (!seq)
            {
                ret->order_ = "[order]:[rand]";
                std::random_shuffle(traverse_order.begin(), traverse_order.end());
            }
            else
            {
                ret->order_ = "[order]:[seq]";
            }
            for (int i = 1; i < unit_num; i++)
            {
                work_ptr[traverse_order.at(i - 1)].next = &work_ptr[traverse_order.at(i)];
            }
            work_ptr[traverse_order.at(unit_num - 1)].next = &work_ptr[traverse_order.at(0)];
        }

        
        if (result.lock.compare_exchange_weak(unlock_stat, lock_stat))
        {
            measure = new util::PmmDataCollector("DIMM data", &imc_read, &imc_write, &media_read, &media_write);
        }
        pthread_barrier_wait(&sync_barrier);
        {
            // run the code
            auto calc_lat = [&]()
            {
                uint64_t start_tick, end_tick;
                start_tick = rdtsc();
                for (int i = 0; i < iter; i++)
                {
                    for (uint64_t j = 0; j < unit_num; j++)
                    {
                        wr_method(work_ptr + traverse_order[j]);
                    }
                }
                end_tick = rdtsc();
                total_cycles = end_tick - start_tick;
            };
            auto chase_ptr_lat = [&]()
            {
                for (int i = 0; i < iter; i++)
                {
                    auto starting_point = work_ptr + random() % unit_num;
                    auto p = starting_point->next;
                    uint64_t start_tick, end_tick;
                    start_tick = rdtsc();
                    while (p != starting_point)
                    {

                        wr_method(&p->pad[15]);
                        p = p->next;
                    }
                    end_tick = rdtsc();
                    total_cycles += (end_tick - start_tick);
                }
            };
            if(trav_tp == CHASING_PTR){
                chase_ptr_lat();
            }else if(trav_tp == CALC_OFFSET){
                calc_lat();
            }
        }

        ret->wss_ = wss;
        ret->avg_cycles_ = total_cycles / iter / unit_num;
    };

    auto run = [&](void (*wr_ops)(void *addr), uint64_t wss, uint64_t iterations, bool seq, int threads, TraverseType traverse_pattern)
    {
        std::vector<FeedBackUnit> results(threads);
        std::vector<std::thread> jobs;
        wr_method = wr_ops;
        result.lock.store(0);
        pthread_barrier_init(&sync_barrier, NULL, threads);
        // run in seq order and rand order

        // run exps

        for (int tid = 0; tid < threads; tid++)
        {
            uint64_t thread_wss = ROUND_DOWN(wss / threads, 256ULL);
            jobs.push_back(std::thread{
                worker,
                addr + tid * thread_wss,
                thread_wss,
                iterations,
                seq,
                traverse_pattern,
                &results.at(tid)});
        }
        // sync threads
        for (int tid = 0; tid < threads; tid++)
        {
            jobs.at(tid).join();
        }
        delete measure;
        measure = NULL;
        // output
        {
            uint64_t lat = 0;
            for (auto item : results)
            {
                lat += item.avg_cycles_;
            }
            std::cout << "-------result--------" << std::endl;
            std::cout << results.at(0).work_type_ << " " << results.at(0).order_ << std::endl;
            std::cout << "[imc wr]:[" << imc_write
                      << "] [imc rd]:[" << imc_read
                      << "] [media wr]:[" << media_write
                      << "] [media rd]:[" << media_read
                      << "] [thread]:[" << threads << "]" << std::endl;
            std::cout << "[wss]:[" << results.at(0).wss_ * threads << "](Bytes) [cycle]: ["
                      << lat / threads
                      << "](CPU cycle)" << std::endl;
            std::cout << "---------------------" << std::endl;
        }
    };
    // for (int thread = 1; thread <= 12; thread++)
    // {
    //     run(rd, (1ULL << 30), 1, false, thread, CHASING_PTR);
    // }

    for (int wss = 12; wss <= 30; wss++)
    {
        run(wr_clwb, (1ULL << wss), (1ULL << 30) / (1ULL << wss), false, 1, CALC_OFFSET);
    }
    for (int wss = 12; wss <= 30; wss++)
    {
        run(wr_nt, (1ULL << wss), (1ULL << 30) / (1ULL << wss), false, 1, CALC_OFFSET);
    }
}

void wr_lat_by_calc(void *addr, uint64_t max_size)
{
    void (*wr_method)(void *);
    memset(&cl_buffer, 0, sizeof(cl_buffer));
    auto func = [&](uint64_t wss, uint64_t iter, bool seq)
    {
        std::cout
            << "-------result--------" << std::endl;
        if (wr_method == rd)
        {
            std::cout << "[type]:[rd]" << std::endl;
        }
        else if (wr_method == wr_nt)
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
        ;
        float imc_read, imc_write, media_read, media_write;
        {
            util::PmmDataCollector measure("DIMM data", &imc_read, &imc_write, &media_read, &media_write);
            start_tick = rdtsc();
            for (int i = 0; i < iter; i++)
            {
                for (uint64_t j = 0; j < node_num; j++)
                {
                    wr_method(work_ptr + order[j]);
                }
            }
            end_tick = rdtsc();
            total_cycles = end_tick - start_tick;
        }
        std::cout << "[imc wr]:[" << imc_write
                  << "] [imc rd]:[" << imc_read
                  << "] [media wr]:[" << media_write
                  << "] [media rd]:[" << media_read
                  << "]" << std::endl;
        std::cout << "[wss]:[" << wss << "](Bytes) [cycle]: ["
                  << total_cycles / iter / node_num
                  << "](CPU cycle)" << std::endl;
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
                func((1ULL << i) + j * (1ULL << (i - 2)), (1ULL << (32 - i)), seq);
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
                func((1ULL << i) + j * (1ULL << (i - 2)), (1ULL << (32 - i)), seq);
            }
        }
    }
    wr_method = rd;
    for (int k = 0; k < 2; k++)
    {
        seq = !seq;
        for (int i = 12; i < 32; i++)
        {
            for (int j = 0; j < 1; j++)
            {
                func((1ULL << i) + j * (1ULL << (i - 2)), (1ULL << (32 - i)), seq);
            }
        }
    }
}