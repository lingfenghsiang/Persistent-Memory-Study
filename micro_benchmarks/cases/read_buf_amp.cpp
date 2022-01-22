#include <istream>
#include "common.h"
#include "json.hpp"

using json = nlohmann::json;

// access pattern
enum increment_style
{
    EXP,
    LINEAR
};

struct Action
{
    uint64_t upper_;
    uint64_t bottom_;
    increment_style incre_;
    uint64_t step_;
};

#define ALIGN_UP(i) ((i + 255ULL) & (~(255ULL)))
#define ALIGN_DOWN(i) (i & (~(255ULL)))

const char work_config[] = RA_WORKCONFIG;


void read_buf_amp_tst(void *addr, uint64_t max_size){
    std::ifstream config_f(work_config);
    std::vector<Action> jobs;
    json j;
    config_f >> j;
    jobs.resize(j["jobs"].size());
    for (int i = 0; i < j["jobs"].size(); i++)
    {
        jobs.at(i).bottom_ = j["jobs"][i]["start_wss"];
        jobs.at(i).bottom_ = ALIGN_DOWN(jobs.at(i).bottom_);
        jobs.at(i).upper_ = j["jobs"][i]["end_wss"];
        jobs.at(i).upper_ = ALIGN_UP(jobs.at(i).upper_);
        jobs.at(i).step_ = j["jobs"][i]["stride"];
        if (j["jobs"][i]["inc_type"] == "exp")
            jobs.at(i).incre_ = EXP;
        else if (j["jobs"][i]["inc_type"] == "linear")
            jobs.at(i).incre_ = LINEAR;
    }

    void (*read_op)(char*);

    auto exec_rd_job=[&](Action task, int cl){
        char *base_addr = (char *)addr;
        
        for (uint64_t i = task.bottom_; i <= task.upper_;)
        {
            std::cout << "-------result--------"<< std::endl;
            std::vector<util::DimmObj> dimm_array;
            // float imc_rd, imc_wr, media_rd, media_write;
            {
                uint64_t iteration = (1ULL << 30) / i;
                util::PmmDataCollector measure("get dimm data", &dimm_array);
                for (int k = 0; k < iteration; )
                {
                    // if

                    for (uint64_t j = 0; j < i; j += 256)
                    {
                        _mm_clflush(addr + j + 0);
                    }
                    if (cl == 1)
                        goto end;
                    for (uint64_t j = 0; j < i; j += 256)
                    {
                        _mm_clflush(addr + j + 64);
                    }
                    if (cl == 2)
                        goto end;
                    for (uint64_t j = 0; j < i; j += 256)
                    {
                        _mm_clflush(addr + j + 128);
                    }
                    if (cl == 3)
                        goto end;
                    for (uint64_t j = 0; j < i; j += 256)
                    {
                        _mm_clflush(addr + j + 192);
                    }
                end:
                    k++;
                }
            }
            _mm_mfence();
            if (cl == 1)
            {
                std::cout
                    << "[type]: [1 line]" << std::endl;
            }
            else if (cl == 2)
            {
                std::cout
                    << "[type]: [2 line]" << std::endl;
            }
            else if (cl == 3)
            {
                std::cout
                    << "[type]: [3 line]" << std::endl;
            }
            else if (cl == 4)
            {
                std::cout
                    << "[type]: [4 line]" << std::endl;
            }
            else
            {
                perror("wrong function");
                exit(EXIT_FAILURE);
            }
            for (auto dimm : dimm_array)
            {
                if (dimm.dimm_id_.compare("0x0100") == 0)
                {
                    std::cout
                        << "[wss]: [" << i << "]\t[RA]:[" << dimm.media_rd / dimm.imc_read << "]" << std::endl;
                    break;
                }
            }

            if (task.incre_ == EXP)
            {
                i = i * task.step_;
            }
            else if (task.incre_ == LINEAR)
            {
                i = i + task.step_;
            }
            std::cout << "---------------------"<< std::endl;
        }

    };
    for (int k = 1; k <= 4; k++)
        for (auto i : jobs)
        {
            exec_rd_job(i, k);
        }
};