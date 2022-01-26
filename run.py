import os
import shutil

this_file_dir = os.path.abspath(os.path.dirname(__file__))
tmp_dir = os.path.join(this_file_dir, "tmp")

python_path = "/home/xlf/anaconda3/bin/python"

if not os.path.isdir(tmp_dir):
    os.makedirs(tmp_dir)

def prepare_case_study():
    src_dir = os.path.join(this_file_dir, "case_study")
    build_dir = os.path.join(this_file_dir, "build_cases")
    if not os.path.isdir(build_dir):
        os.makedirs(build_dir)
    build_cmd = "cd " + build_dir + " && cmake " + src_dir + " && make -j"
    os.system(build_cmd)

    if not os.path.exists(os.path.join(tmp_dir, "ycsb-0.17.0.tar.gz")):
        ycsb_download_cmd = "cd "+ tmp_dir +" && wget https://github.com/brianfrankcooper/YCSB/releases/download/0.17.0/ycsb-0.17.0.tar.gz"
        os.system(ycsb_download_cmd)
    if not os.path.exists(os.path.join(tmp_dir, "ycsb-0.17.0")):
        os.system("cd "+ tmp_dir +" && tar -xvf " + os.path.join(tmp_dir, "ycsb-0.17.0.tar.gz"))

    prepare_workload_cmd = python_path + " " + os.path.join(this_file_dir, "tools", "generate_workload.py") + " -op_num=5000000"
    print(prepare_workload_cmd)
    os.system(prepare_workload_cmd)

def run_case_study(max_worker, pmem_directory):
    build_dir = os.path.join(this_file_dir, "build_cases")
    # fastfair test
    def fast_fair_test():
        for i in ["fastfair_original_test", "fastfair_rap_mod_test"]:
            init_command = "echo >" + os.path.join(tmp_dir,  i + ".log")
            os.system(init_command)
            for j in range(1, max_worker + 1):
                run_cmd = "numactl -N 0 " +os.path.join(build_dir, i) + " -thread " + str(j) + " >> " + os.path.join(tmp_dir,  i + ".log")
                os.system(run_cmd)
    fast_fair_test()
    

    # cceh test
    def cceh_test(on_pm:bool = True):
        if on_pm:
            pmem_dir = pmem_directory
            media_type = "pmm"
        else:
            pmem_dir = "/dev/shm/"
            media_type = "dram"
        init_command = "echo >" + os.path.join(tmp_dir,  "cceh_" + media_type + ".log")
        os.system(init_command)
        for i in range(1, max_worker + 1):
            run_cmd = "numactl -N 0 " + os.path.join(build_dir, "cceh_test") + " -pool_dir " + pmem_dir \
                + " -thread " + \
                str(i) + " >> " + os.path.join(tmp_dir,
                                               "cceh_" + media_type + ".log")
            print("\n" + run_cmd + "\n")
            os.system(run_cmd)
            os.remove(os.path.join(os.path.abspath(pmem_dir), "cceh_pmempool"))
        init_command = "echo >" + \
            os.path.join(tmp_dir,  "cceh_preread_" + media_type + ".log")
        os.system(init_command)
        for i in range(1, max_worker + 1):
            run_cmd = "numactl -N 0 " + os.path.join(build_dir, "cceh_test") + " -pool_dir " + pmem_dir \
                + " -preread -thread " + \
                str(i) + " >> " + os.path.join(tmp_dir,
                                               "cceh_preread_" + media_type + ".log")
            os.system(run_cmd)
            os.remove(os.path.join(os.path.abspath(pmem_dir), "cceh_pmempool"))
    cceh_test(True)
    cceh_test(False)

def prepare_microbench():
    src_dir = os.path.join(this_file_dir, "micro_benchmarks")
    build_dir = os.path.join(this_file_dir, "build_benchmark")
    if not os.path.isdir(build_dir):
        os.makedirs(build_dir)
    build_cmd = "cd " + build_dir + " && cmake " + src_dir + " && make -j"
    os.system(build_cmd)

def run_microbench_except_prefetching():
    build_dir = os.path.join(this_file_dir, "build_benchmark", "bin")

    init_command = "echo >" + os.path.join(tmp_dir,  "task0.log")
    os.system(init_command)
    rd_amp_run_cmd = "numactl -N 0 " + os.path.join(build_dir, "microbench") + " -test 0 >> " + os.path.join(tmp_dir,  "task0.log")
    os.system(rd_amp_run_cmd)

    init_command = "echo >" + os.path.join(tmp_dir,  "task2.log")
    os.system(init_command)
    wr_buf_run_cmd = "numactl -N 0 " + os.path.join(build_dir, "microbench") + " -test 2 >> " + os.path.join(tmp_dir,  "task2.log")
    os.system(wr_buf_run_cmd)

    init_command = "echo >" + os.path.join(tmp_dir,  "task4.log")
    os.system(init_command)
    seperate_rd_wr_buf_cmd = "numactl -N 0 " + os.path.join(build_dir, "microbench") + " -test 4 >> " + os.path.join(tmp_dir,  "task4.log")
    os.system(seperate_rd_wr_buf_cmd)

    init_command = "echo >" + os.path.join(tmp_dir,  "task5.log")
    os.system(init_command)
    rap_cmd = "numactl -N 0 " + os.path.join(build_dir, "microbench") + " -test 5 >> " + os.path.join(tmp_dir,  "task5.log")
    os.system(rap_cmd)

    init_command = "echo >" + os.path.join(tmp_dir,  "task5_dram.log")
    os.system(init_command)
    rap_cmd = "numactl -N 0 " + os.path.join(build_dir, "microbench") + " -nopmm -test 5 >> " + os.path.join(tmp_dir,  "task5_dram.log")
    os.system(rap_cmd)

    init_command = "echo >" + os.path.join(tmp_dir,  "task6.log")
    os.system(init_command)
    access_lat_cmd = "numactl -N 0 " + os.path.join(build_dir, "microbench") + " -test 6 >> " + os.path.join(tmp_dir,  "task6.log")
    print(access_lat_cmd)
    os.system(access_lat_cmd)

def run_microbench_prefetching():
    build_dir = os.path.join(this_file_dir, "build_benchmark", "bin")
    init_command = "echo >" + os.path.join(tmp_dir,  "task1.log")
    os.system(init_command)
    prefetching_cmd = os.path.join(build_dir, "microbench") + " -test 1 >> " + os.path.join(tmp_dir,  "task1.log")
    os.system(prefetching_cmd)


def format_logs():
    # prepare folders
    output_dir = os.path.join(this_file_dir, "output")
    if not os.path.isdir(output_dir):
        os.makedirs(output_dir)
    case_study_dir = os.path.join(output_dir, "case_study")
    if not os.path.isdir(case_study_dir):
        os.makedirs(case_study_dir)
    micro_bench_dir = os.path.join(output_dir, "micro_bench")
    if not os.path.isdir(micro_bench_dir):
        os.makedirs(micro_bench_dir)
    tool_path = os.path.join(this_file_dir, "tools", "format_log.py")
    os.system(python_path + " " + tool_path +
              " -log_path=" + os.path.join(tmp_dir, "fastfair_original_test.log") +
              " -config_path=" + os.path.join(this_file_dir, "tools", "case_study_config0.json") +
              " -tmp_dir=" + tmp_dir +
              " -out_dir=" + case_study_dir)
    os.system(python_path + " " + tool_path +
              " -log_path=" + os.path.join(tmp_dir, "fastfair_rap_mod_test.log") +
              " -config_path=" + os.path.join(this_file_dir, "tools", "case_study_config1.json") +
              " -tmp_dir=" + tmp_dir +
              " -out_dir=" + case_study_dir)
    os.system(python_path + " " + tool_path +
              " -log_path=" + os.path.join(tmp_dir, "cceh_pmm.log") +
              " -config_path=" + os.path.join(this_file_dir, "tools", "case_study_config2.json") +
              " -tmp_dir=" + tmp_dir +
              " -out_dir=" + case_study_dir)
    os.system(python_path + " " + tool_path +
              " -log_path=" + os.path.join(tmp_dir, "cceh_preread_pmm.log") +
              " -config_path=" + os.path.join(this_file_dir, "tools", "case_study_config3.json") +
              " -tmp_dir=" + tmp_dir +
              " -out_dir=" + case_study_dir)
    os.system(python_path + " " + tool_path +
              " -log_path=" + os.path.join(tmp_dir, "cceh_dram.log") +
              " -config_path=" + os.path.join(this_file_dir, "tools", "case_study_config4.json") +
              " -tmp_dir=" + tmp_dir +
              " -out_dir=" + case_study_dir)
    os.system(python_path + " " + tool_path +
              " -log_path=" + os.path.join(tmp_dir, "cceh_preread_dram.log") +
              " -config_path=" + os.path.join(this_file_dir, "tools", "case_study_config5.json") +
              " -tmp_dir=" + tmp_dir +
              " -out_dir=" + case_study_dir)

    os.system(python_path + " " + tool_path +
              " -log_path=" + os.path.join(tmp_dir, "task0.log") +
              " -config_path=" + os.path.join(this_file_dir, "tools", "microbench_config0.json") +
              " -tmp_dir=" + tmp_dir +
              " -out_dir=" + micro_bench_dir)
    
    os.system(python_path + " " + tool_path +
            " -log_path=" + os.path.join(tmp_dir, "task1.log") +
            " -config_path=" + os.path.join(this_file_dir, "tools", "microbench_config1.json") +
            " -tmp_dir=" + tmp_dir +
            " -out_dir=" + micro_bench_dir)

    os.system(python_path + " " + tool_path +
              " -log_path=" + os.path.join(tmp_dir, "task2.log") +
              " -config_path=" + os.path.join(this_file_dir, "tools", "microbench_config2.json") +
              " -tmp_dir=" + tmp_dir +
              " -out_dir=" + micro_bench_dir)

    # task4 to test seperate buffer does not need a graph

    os.system(python_path + " " + tool_path +
              " -log_path=" + os.path.join(tmp_dir, "task5.log") +
              " -config_path=" + os.path.join(this_file_dir, "tools", "microbench_config5_0.json") +
              " -tmp_dir=" + tmp_dir +
              " -out_dir=" + micro_bench_dir)
    os.system(python_path + " " + tool_path +
              " -log_path=" + os.path.join(tmp_dir, "task5_dram.log") +
              " -config_path=" + os.path.join(this_file_dir, "tools", "microbench_config5_1.json") +
              " -tmp_dir=" + tmp_dir +
              " -out_dir=" + micro_bench_dir)              
    os.system(python_path + " " + tool_path +
              " -log_path=" + os.path.join(tmp_dir, "task6.log") +
              " -config_path=" + os.path.join(this_file_dir, "tools", "microbench_config6.json") +
              " -tmp_dir=" + tmp_dir +
              " -out_dir=" + micro_bench_dir)

def plot_results():
    # plot microbench
    def run_plot_script(file_name):
        tool_path = os.path.join(this_file_dir, "tools", file_name)
        os.system(python_path + " " + tool_path)
    shutil.copy(os.path.join(tmp_dir, "task2.log"), os.path.join(this_file_dir, "output", "micro_bench", "seperate_rd_wr_buf.log"))
    run_plot_script("plot_bench_lat.py")
    run_plot_script("plot_bench_prefetching.py")
    run_plot_script("plot_bench_rd_amp.py")
    run_plot_script("plot_bench_read_after_persist.py")
    run_plot_script("plot_bench_wr_buf.py")
    run_plot_script("plot_case_btree.py")
    run_plot_script("plot_case_cceh.py")


prepare_case_study()
run_case_study(6, "/mnt/pmem/")

prepare_microbench()
run_microbench_except_prefetching()
run_microbench_prefetching()

format_logs()
plot_results()

