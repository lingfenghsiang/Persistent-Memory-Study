#include "common.h"

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
        std::cout << "---------------------"<< std::endl;
        user_result_dummy += sum;
    };
    wr_op = wr_nt_mfence;
    for (int i = 0; i < 70; i++)
    {
        func(1ULL << 13, 50000, i);
    }
    wr_op = wr_clwb_mfence;
    for (int i = 0; i < 70; i++)
    {
        func(1ULL << 13, 50000, i);
    }
    wr_op = wr_nt_sfence;
    for (int i = 0; i < 70; i++)
    {
        func(1ULL << 13, 50000, i);
    }
    wr_op = wr_clwb_sfence;
    for (int i = 0; i < 70; i++)
    {
        func(1ULL << 13, 50000, i);
    }
}
