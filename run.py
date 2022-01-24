import glob
import os
import shutil

this_file_dir = os.path.abspath(os.path.dirname(__file__))
tmp_dir = os.path.join(this_file_dir, "tmp")
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

    prepare_workload_cmd = "python3 " + os.path.join(this_file_dir, "tools", "generate_workload.py") + " -op_num=5000000"
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
            file_in_pmem_dir = glob.glob(os.path.abspath(pmem_dir) + "/*")
            for i in file_in_pmem_dir:
                shutil.rmtree(i)
    # cceh test
    init_command = "echo >" + os.path.join(tmp_dir,  "cceh.log")
    for i in range(1, max_worker + 1):
        run_cmd = "numactl -N 0 " + os.path.join(build_dir, "cceh_test") + " -thread " + str(i) + " >> " + os.path.join(tmp_dir,  "cceh.log")
        os.system(run_cmd)
        file_in_pmem_dir = glob.glob(os.path.abspath(pmem_dir) + "/*")
        for i in file_in_pmem_dir:
            shutil.rmtree(i)
    init_command = "echo >" + os.path.join(tmp_dir,  "cceh_prepread.log")
    for i in range(1, max_worker + 1):    
        run_cmd = "numactl -N 0 " + os.path.join(build_dir, "cceh_test") + " -preread -thread " + str(i) + " >> " + os.path.join(tmp_dir,  "cceh_prepread.log")
        os.system(run_cmd)
        file_in_pmem_dir = glob.glob(os.path.abspath(pmem_dir) + "/*")
        for i in file_in_pmem_dir:
            shutil.rmtree(i)

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

# prepare_case_study()
run_case_study(8, "/mnt/pmem")

# prepare_microbench()