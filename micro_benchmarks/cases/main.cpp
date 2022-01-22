#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "common.h"
#include <x86intrin.h>
#include <sstream>
#include <gflags/gflags.h>
#include <numa.h>

VMEM *vmp;
cpu_set_t native_cpuset[HYPER_THERAD_CORE_NUM / SOCKET_NUM];
// access pattern
motherboard_cpu_layout *cpu_info_ptr = (motherboard_cpu_layout *)cpus_on_board;


#define ALIGN_DOWN(addr) ((addr & (~(0xfffULL))))

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

DEFINE_uint64(max_size, PERSISTENT_MAP_SIZE, "maximum_pool_size in bytes");
DEFINE_string(pool_dir, PERSISTENT_POOL_DIR, "where to put the pool file");
DEFINE_bool(pmm, true, "shall we run the test on pm?");
DEFINE_int32(numa_node, 0, "which numa node to run on");
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
    // if (numa_available())
    // {
    //     std::cout << "numa is available" << std::endl;
    // }
    // numa_set_preferred(FLAGS_numa_node);

    cpu_set_t local_cpu;
    CPU_ZERO(&local_cpu);
    CPU_SET(8, &local_cpu);

    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &local_cpu);

    // get physical address
    auto pid = getpid();
    std::string pid_str = std::to_string(pid);
    std::string va_low;
    std::stringstream sstream;
    sstream << std::hex << test_addr_namespace;
    std::string hex_addr;
    sstream >> hex_addr;
    std::string cmd(GET_PAGETABLE);
    cmd = cmd + pid_str + " " + hex_addr;
    sstream.clear();
    sstream << std::hex << test_addr_namespace + 4096;
    sstream >> hex_addr;
    cmd = cmd + " " + hex_addr;
    std::cout << cmd << std::endl;
    system(cmd.c_str());

    // test code put here
    auto uncachable_offset = 0;
    void (*test_funcs[])(void *addr, uint64_t max_size) = {
        read_after_flush_lock_contention, //0
        read_after_flush,                 //1
        read_buf_partial,                 //2
        trigger_prefetching,              //3
        extreme_case_prefetching,         //4
        write_buffer,                     //5
        write_buffer_flushing_period,     //6
        seperate_rd_wr_buf,               //7
        read_and_write_bw,                //8
        access_lat,                       //9
        wr_lat_by_calc,                   //10
        wr_bw_by_calc,                    //11
        read_buf_amp_tst,                 //12
        lat_test,                         //13
        write_buffer_size,                //14
        veri_rmw                          //15
    };

    test_funcs[9](test_addr_namespace + uncachable_offset, FLAGS_max_size - uncachable_offset);

    // vmem_free(vmp, test_addr_namespace);
    // vmem_delete(vmp);
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