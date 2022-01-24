import glob
import os
import shutil

this_file_dir = os.path.abspath(os.path.dirname(__file__))
tmp_dir = os.path.join(this_file_dir, "tmp")

python_path = "/home/xlf/anaconda3/bin/python"

if not os.path.isdir(tmp_dir):
    os.system(tmp_dir)

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

def run_case_study(max_worker, pmem_dir):
    build_dir = os.path.join(this_file_dir, "build_cases")
    # fastfair test
    for i in ["fastfair_original_test", "fastfair_rap_mod_test"]:
        init_command = "echo >" + os.path.join(tmp_dir,  i + ".log")
        os.system(init_command)
        for j in range(1, max_worker + 1):
            run_cmd = "numactl -N 0 " +os.path.join(build_dir, i) + " -thread " + str(j) + " >> " + os.path.join(tmp_dir,  i + ".log")
            os.system(run_cmd)

    # cceh test
    init_command = "echo >" + os.path.join(tmp_dir,  "cceh.log")
    for i in range(1, max_worker + 1):
        run_cmd = "numactl -N 0 " + os.path.join(build_dir, "cceh_test") + " -thread " + str(i) + " >> " + os.path.join(tmp_dir,  "cceh.log")
        os.system(run_cmd)
        os.remove(os.path.join(os.path.abspath(pmem_dir ), "cceh_pmempool"))
    init_command = "echo >" + os.path.join(tmp_dir,  "cceh_prepread.log")
    for i in range(1, max_worker + 1):    
        run_cmd = "numactl -N 0 " + os.path.join(build_dir, "cceh_test") + " -preread -thread " + str(i) + " >> " + os.path.join(tmp_dir,  "cceh_prepread.log")
        os.system(run_cmd)
        os.remove(os.path.join(os.path.abspath(pmem_dir ), "cceh_pmempool"))

def prepare_microbench():
    src_dir = os.path.join(this_file_dir, "micro_benchmarks")
    build_dir = os.path.join(this_file_dir, "build_benchmark")
    if not os.path.isdir(build_dir):
        os.makedirs(build_dir)
    build_cmd = "cd " + build_dir + " && cmake " + src_dir + " && make -j"
    os.system(build_cmd)

def run_microbench():
    build_dir = os.path.join(this_file_dir, "build_benchmark", "bin")
    init_command = "echo >" + os.path.join(tmp_dir,  "task0.log")
    os.system(init_command)
    rd_amp_run_cmd = os.path.join(build_dir, "microbench") + " -test 0 >> " + os.path.join(tmp_dir,  "task0.log")

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
              " -log_path=" + os.path.join(tmp_dir, "cceh.log") +
              " -config_path=" + os.path.join(this_file_dir, "tools", "case_study_config2.json") +
              " -tmp_dir=" + tmp_dir +
              " -out_dir=" + case_study_dir)
    os.system(python_path + " " + tool_path +
              " -log_path=" + os.path.join(tmp_dir, "cceh_prepread.log") +
              " -config_path=" + os.path.join(this_file_dir, "tools", "case_study_config3.json") +
              " -tmp_dir=" + tmp_dir +
              " -out_dir=" + case_study_dir)


# prepare_case_study()
# run_case_study(8, "/mnt/pmem")
format_logs()
# prepare_microbench()
