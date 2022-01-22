#include "common.h"
#include <x86intrin.h>
#include <algorithm>

static int user_result_dummy;
void read_buf_partial(void *addr, uint64_t max_size)
{
    register int sum = 0;
    int iterations = 1ULL << 30;
    uint64_t size = 1ULL<<12;
    if (size >= max_size)
    {
        perror("too large wss");
        return;
    }
    std::vector<uint32_t> sequence(size>>8);
    for(int i = 0;i < sequence.size(); i ++){
        sequence.at(i) = i;
    }
    std::random_shuffle(sequence.begin(), sequence.end());

    auto starter = std::chrono::system_clock::now();
    int *lastone = (int *)(addr + size);
    int *p;
    while (iterations-- > 0)
    {
        for (auto offest : sequence)
        {
            p = ((int *)addr) + (offest << 6);
#define DOIT(i) volatile auto buff##i = _mm512_stream_load_si512((void *)(p + i));
#include "op_256B.h"
#undef DOIT
        }
    }
    user_result_dummy += sum;
    auto ender = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(ender-starter);
    std::cout<< "time is "<< duration.count() << "ms" << std::endl;
}


