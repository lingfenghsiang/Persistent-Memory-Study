#include "./common.h"
#include "xlf_util.h"

#define ALIGN_UP(i) ((i + 255ULL) & (~(255ULL)))
#define ALIGN_DOWN(i) (i & (~(255ULL)))

VMEM *vmp;
cpu_set_t native_cpuset[HYPER_THERAD_CORE_NUM / SOCKET_NUM];
// access pattern
enum increment_style
{
    EXP,
    LINEAR
};
motherboard_cpu_layout *cpu_info_ptr = (motherboard_cpu_layout *)cpus_on_board;

struct Action
{
    uint64_t upper_;
    uint64_t bottom_;
    increment_style incre_;
    uint64_t step_;
};

int main(int argc, char **argv)
{
    std::cout << "---------------------Test start-----------------------" << std::endl;
    // default parameters
    std::vector<Action> jobs;
    uint64_t max_wss = 0;
    uint64_t iteration = 1 << 14ULL;

    // argument parser
    std::string incoming_string;
    for (int i = 1; i < argc; i++)
    {
        incoming_string = argv[i];
        if (incoming_string.compare("-a") == 0)
        {
            Action instance;
            i++;
            if (i == argc)
            {
                std::cout << "please specify action" << std::endl;
                exit(0);
            }
            incoming_string = argv[i];
            int j = incoming_string.length() - 1;
            for (; j >= 0; j--)
            {
                if (std::isdigit(incoming_string.at(j)) == 0)
                {
                    std::cout << "illegal input" << std::endl;
                    exit(0);
                };
            }
            instance.bottom_ = std::stoull(incoming_string);

            i++;
            if (i == argc)
            {
                std::cout << "please specify action" << std::endl;
                exit(0);
            }
            incoming_string = argv[i];
            j = incoming_string.length() - 1;
            for (; j >= 0; j--)
            {
                if (std::isdigit(incoming_string.at(j)) == 0)
                {
                    std::cout << "illegal input" << std::endl;
                    exit(0);
                };
            }
            instance.upper_ = std::stoull(incoming_string);

            i++;
            if (i == argc)
            {
                std::cout << "please specify action" << std::endl;
                exit(0);
            }
            incoming_string = argv[i];

            if (incoming_string.compare("linear") == 0)
            {
                instance.incre_ = LINEAR;
            }
            else if (incoming_string.compare("exp") == 0)
            {
                instance.incre_ = EXP;
            }
            else
            {
                std::cout << "illegal input" << std::endl;
                exit(0);
            }

            i++;
            if (i == argc)
            {
                std::cout << "please specify action" << std::endl;
                exit(0);
            }
            incoming_string = argv[i];
            j = incoming_string.length() - 1;
            for (; j >= 0; j--)
            {
                if (std::isdigit(incoming_string.at(j)) == 0)
                {
                    std::cout << "illegal input" << std::endl;
                    exit(0);
                };
            }
            instance.step_ = std::stoull(incoming_string);
            instance.bottom_ = ALIGN_DOWN(instance.bottom_);
            instance.upper_ = ALIGN_UP(instance.upper_);
            if (instance.bottom_ == 0 || instance.upper_ <= instance.bottom_)
            {
                std::cout << "illegal range" << std::endl;
                exit(0);
            }
            if (instance.incre_ == LINEAR)
            {
                instance.step_ = ALIGN_UP(instance.step_);
            }
            jobs.push_back(instance);
        }
        else
        {
            std::cout << "illegal parameter detected, you need to specify actions:" << std::endl;
            std::cout << "-a for actions, followed by lower bound, upper bound, linear and step. Bound will be aligned to 256B" << std::endl;
            std::cout << "e.g. -a 1024 2048 linear 256" << std::endl;
        }
    }

    for (auto i : jobs)
    {
        if (max_wss < i.upper_)
            max_wss = i.upper_;
        std::cout << i.bottom_ << "B to " << i.upper_ << "B ";
        if (i.incre_ == LINEAR)
        {
            std::cout << "linearly, " << i.step_ << "B" << std::endl;
        }
        else if (i.incre_ == EXP)
        {
            std::cout << "exponentially, step *" << i.step_ << std::endl;
        };
    }

    // bind cores

    for (int j = 0; j < HYPER_THERAD_CORE_NUM / PHY_CORE_NUM; j++)
    {
        for (int i = 0; i < PHY_CORE_NUM / SOCKET_NUM; i++)
        {
            CPU_ZERO(native_cpuset + j * PHY_CORE_NUM / SOCKET_NUM + i);
            CPU_SET(cpu_info_ptr->cpu_socket_[0].cpu_physical_[i].hyperthread_core_[j],
                    native_cpuset + j * PHY_CORE_NUM / SOCKET_NUM + i);
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
    if ((test_addr_namespace = vmem_aligned_alloc(vmp, 256, max_wss)) == NULL)
    {
        perror("vmem_malloc");
        exit(1);
    }
    memset(test_addr_namespace, 0, (max_wss));
    auto detect_buffer_size = [&](Action job_instance) {
        char *base_addr = (char *)test_addr_namespace;
        for (uint64_t i = job_instance.bottom_; i <= job_instance.upper_;)
        {
            float imc_rd, imc_wr, media_rd, media_write;
            {
                util::PmmDataCollector measure("get dimm data", &imc_rd, &imc_wr, &media_rd, &media_write);
                for (int k = 0; k < iteration; k++)
                {
                    // read cacheline 0
                    for (uint64_t j = 0; j < i; j += 256)
                    {
                        _mm_clflush(base_addr + j + 0);
                    }
                    // read cacheline 1
                    for (uint64_t j = 0; j < i; j += 256)
                    {
                        _mm_clflush(base_addr + j + 64);
                    }
                    // read cacheline 2
                    for (uint64_t j = 0; j < i; j += 256)
                    {
                        _mm_clflush(base_addr + j + 128);
                    }
                    // read cacheline 3
                    for (uint64_t j = 0; j < i; j += 256)
                    {
                        _mm_clflush(base_addr + j + 192);
                    }
                }
            }
            _mm_mfence();
            std::cout << "wss: " << i << "\tRA:" << media_rd / imc_rd << std::endl;
            if (job_instance.incre_ == EXP)
            {
                i = i * job_instance.step_;
            }
            else if (job_instance.incre_ == LINEAR)
            {
                i = i + job_instance.step_;
            }
        }
    };

    auto read_same_cacheline = [&](Action job_instance) {
        char *base_addr = (char *)test_addr_namespace;
        for (uint64_t i = job_instance.bottom_; i <= job_instance.upper_;)
        {
            float imc_rd, imc_wr, media_rd, media_write;
            {
                util::PmmDataCollector measure("get dimm data", &imc_rd, &imc_wr, &media_rd, &media_write);
                for (int k = 0; k < iteration; k++)
                {
                    // read cacheline 0
                    for (uint64_t j = 0; j < i; j += 256)
                    {
                        _mm_clflush(base_addr + j + 0);
                    }
                }
            }
            _mm_mfence();
            std::cout << "wss: " << i << "\tRA:" << media_rd / imc_rd << std::endl;
            if (job_instance.incre_ == EXP)
            {
                i = i * job_instance.step_;
            }
            else if (job_instance.incre_ == LINEAR)
            {
                i = i + job_instance.step_;
            }
        }
    };

    std::cout << "---------------------Test 0-----------------------" << std::endl;
    // read different cacheline
    for (auto i : jobs)
        detect_buffer_size(i);
    // read same cacheline
    std::cout << "---------------------Test 1-----------------------" << std::endl;
    for (auto i : jobs)
        read_same_cacheline(i);
    vmem_free(vmp, test_addr_namespace);
    vmem_delete(vmp);
    std::cout << "---------------------Test over-----------------------" << std::endl;
    return 0;
}