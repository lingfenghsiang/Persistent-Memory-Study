#include <fstream>
#include <iostream>
#include <vector>
#include <atomic>
#include <algorithm>
#include <gflags/gflags.h>
#include <unordered_map>
#include "tests.hpp"

#include "include/cceh_pmdk.hpp"
#include "include/fastfair.hpp"

DEFINE_string(loadfile, LOAD_FILE_DIR, "ycsb load file");
DEFINE_string(runfile, RUN_FILE_DIR, "ycsb run file");
DEFINE_string(pool_dir, PMEM_POOL_DIR, "perisist pool to put data structure");
DEFINE_uint64(pool_size, PMEM_POOL_SIZE, "perisist pool size");
DEFINE_uint64(thread, 1, "number of working threads");
DEFINE_bool(preread, false, "do we enable preread?");
DEFINE_int32(prereadnum, 4, "number of preread kv pairs");

const int init_size = 4096;

int main(int argc, char *argv[])
{
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    std::ifstream infile;
    std::string op, key;
    std::vector<std::pair< uint64_t, uint64_t >> init_kv;
    std::vector<operation> init_ops;
    std::vector<std::pair< uint64_t, uint64_t >> run_kv;
    std::vector<operation> runtime_ops;
    init_kv.reserve(init_size);
    init_ops.reserve(init_size);
    run_kv.reserve(init_size);
    runtime_ops.reserve(init_size);

    {
        // parepare warm-up data
        int max_size = init_size;
        infile.open(FLAGS_loadfile);
        int i = 0;
        while ((infile >> op >> key))
        {
            init_kv.push_back(std::make_pair(strtoull(key.c_str(), nullptr, 10), strtoull(key.c_str(), nullptr, 10)));
            init_ops.push_back(operation::INSERT);
            i++;
            if (i == max_size)
            {
                max_size *= 2;
                init_kv.reserve(max_size - init_kv.size());
                init_ops.reserve(max_size - init_ops.size());
            }
        }
        infile.close();
        std::cout << i << " keys initialized" << std::endl;
    }

    {
        // prepare run data
        int max_size = init_size;
        infile.open(FLAGS_runfile);
        int i = 0;
        while ((infile >> op >> key))
        {
            run_kv.push_back(std::make_pair(strtoull(key.c_str(), nullptr, 10), strtoull(key.c_str(), nullptr, 10)));
            if (op.compare("INSERT") == 0)
            {
                runtime_ops.push_back(operation::INSERT);
            }
            else if (op.compare("READ") == 0)
            {
                runtime_ops.push_back(operation::READ);
            }
            else if (op.compare("UPDATE") == 0)
            {
                runtime_ops.push_back(operation::UPDATE);
            }
            else if (op.compare("DELETE") == 0)
            {
                runtime_ops.push_back(operation::DELETE);
            }
            else
            {
                perror("illegal operation");
                exit(EXIT_FAILURE);
            }
            i++;
            if (i == max_size)
            {
                max_size *= 2;
                run_kv.reserve(max_size - run_kv.size());
                runtime_ops.reserve(max_size - runtime_ops.size());
            }
        }

        infile.close();
        std::cout << i << " ops initialized" << std::endl;
    }
#ifdef CCEH_TST
    ccehpmdk::cceh pmdk_cceh;
    pmdk_cceh.Init(FLAGS_pool_dir, FLAGS_pool_size);
    int j = 0;
    for(auto i:init_kv){
        j++;
        pmdk_cceh.Insert(i.first, i.second);
        if (j%100==0){std::cout <<j<<std::endl;}
    }

    // test<uint64_t, uint64_t>(pmdk_cceh, init_kv, init_ops, FLAGS_thread, FLAGS_preread, FLAGS_prereadnum, "load phase");
    // test<uint64_t, uint64_t>(pmdk_cceh, run_kv, runtime_ops, FLAGS_thread, FLAGS_preread, FLAGS_prereadnum, "run phase");
    pmdk_cceh.CleanUp();
#endif
#ifdef FASTFAIR_TST
    fast_fair::fast_fair fastfair_obj;
    fastfair_obj.Init(FLAGS_pool_dir, FLAGS_pool_size);
    test<uint64_t, uint64_t>(fastfair_obj, init_kv, init_ops, FLAGS_thread, FLAGS_preread, FLAGS_prereadnum, "load phase");
    test<uint64_t, uint64_t>(fastfair_obj, run_kv, runtime_ops, FLAGS_thread, FLAGS_preread, FLAGS_prereadnum, "run phase");
    fastfair_obj.CleanUp();
#endif
    return 0;
}