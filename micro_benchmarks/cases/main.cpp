#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "common.h"
#include <x86intrin.h>
#include <sstream>
#include <gflags/gflags.h>

void nt_bzero(void *addr, uint64_t size)
{
    __m512i zero;
    char *aligned_addr = (char *)addr;
    memset(&zero, -1, sizeof(__m512d));
    for (; size > 0;)
    {
        _mm512_stream_si512((__m512i *)aligned_addr, zero);
        aligned_addr += 256;
        size -= 256;
    }
}

DEFINE_string(pool_dir, PERSISTENT_POOL_DIR, "where to put the pool file");
DEFINE_uint64(max_size, PERSISTENT_MAP_SIZE, "maximum_pool_size in bytes");
DEFINE_bool(pmm, true, "shall we run the test on pm?");
DEFINE_int32(test, 2, "tests to run, available tests include:\n\
0. Read amplification test.\n\
1. Prefetching test.\n\
2. write buffer test.\n\
3. write buffer flushing period test. ");

int main(int argc, char **argv)
{
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    std::cout << "---------------------Test start-----------------------" << std::endl;

    void *test_addr_namespace;
    // mmap persist address
    if (FLAGS_pmm)
    {
        int fd;
        if ((fd = open(FLAGS_pool_dir.c_str(), O_CREAT | O_RDWR, 0644)) < 0)
        {
            printf("Open file failed!, errno is %d\n", errno);
            exit(-1);
        }
        lseek(fd, FLAGS_max_size, SEEK_SET);
        write(fd, "", 1);
        if ((test_addr_namespace = mmap(NULL, FLAGS_max_size, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0)) < 0)
        {
            perror("malloc error");
            exit(1);
        }
        close(fd);
        nt_bzero(test_addr_namespace, FLAGS_max_size);
    }
    else
    {
        posix_memalign(&test_addr_namespace, 64, FLAGS_max_size);
    }

    // test code put here
    auto uncachable_offset = 0;
    void (*test_funcs[])(void *addr, uint64_t max_size) = {
        read_buf_amp_tst,                  // 0
        trigger_prefetching,               // 1
        write_buffer,                      // 2
        write_buffer_flushing_period,      // 3
        seperate_rd_wr_buf,                // 4
        read_after_flush,                  // 5
        access_lat,                        // 6
        read_after_flush_lock_contention,  // 7
        extreme_case_prefetching,          // 8
        rd_throughput_against_prefetching, // 9
    };

    test_funcs[FLAGS_test](test_addr_namespace + uncachable_offset, FLAGS_max_size - uncachable_offset);

    std::cout << "---------------------Test over-----------------------" << std::endl;
    if (FLAGS_pmm)
    {
        if ((munmap(test_addr_namespace, FLAGS_max_size)) == -1)
        {
            printf("Munmap error\n");
            exit(-1);
        }
    }
    else
    {
        free(test_addr_namespace);
    }

    return 0;
}