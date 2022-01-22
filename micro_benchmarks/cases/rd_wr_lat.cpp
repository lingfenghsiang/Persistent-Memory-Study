#include "common.h"

#define NPAD 7

struct AccessUnit
{
    struct AccessUnit *next;
    uint64_t pad[NPAD];
};

void lat_test(void *addr, uint64_t max_size)
{

    auto func = [&](void *local_addr, uint64_t working_set_size, int iterations, bool rand_order)
    {
        AccessUnit *work_ptr = (AccessUnit *)local_addr, *tmp_ptr, *start_ptr;
        uint64_t unit_num = working_set_size / sizeof(AccessUnit);
        uint64_t tic, tok, timer = 0;
        std::vector<uint64_t> order(unit_num);
        for (uint64_t i = 0; i < unit_num; i++)
        {
            order.at(i) = i;
        }
        if (rand_order)
            std::random_shuffle(order.begin(), order.end());
        for (uint64_t i = 1; i < unit_num; i++)
        {
            work_ptr[order.at(i - 1)].next = &work_ptr[order.at(i)];
        }
        work_ptr[order.at(unit_num - 1)].next = &work_ptr[order.at(0)];

        for (int i = 0; i < iterations; i++)
        {
            start_ptr = work_ptr + rand() % unit_num;
            tmp_ptr = start_ptr->next;
            tic = rdtsc();
            while (tmp_ptr != start_ptr)
            {
                tmp_ptr = tmp_ptr->next;
                // tmp_ptr->pad[15] = 10;
                // _mm_clwb(&tmp_ptr->pad[15]);
            }
            tok = rdtsc();
            timer += tok - tic;
        }
        std::cout << "-------result--------" << std::endl;
        if (rand_order)
        {
            std::cout << "[order]:[rand]" << std::endl;
        }
        else
        {
            std::cout << "[order]:[seq]" << std::endl;
        }
        std::cout << "[wss]:[" << working_set_size << "](B)" << std::endl;
        std::cout << "[latency]:[" << timer / iterations / unit_num << "](cycles)" << std::endl;
        std::cout << "---------------------" << std::endl;
    };
    bool rand_order = true;
    for (int i = 8; i < 31; i++)
    {
        func(addr, (1ULL << i), (1ULL << (31 - i)), rand_order);
    }
    rand_order = false;
    for (int i = 8; i < 31; i++)
    {
        func(addr, (1ULL << i), (1ULL << (31 - i)), rand_order);
    }
}
    