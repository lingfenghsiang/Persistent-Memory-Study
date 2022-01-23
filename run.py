from multiprocessing.spawn import prepare
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

    prepare_workload_cmd = "python3 " + os.path.join(this_file_dir, "tools", "generate_workload.py") + " -op_num=8000"
    print(prepare_workload_cmd)
    os.system(prepare_workload_cmd)
prepare_case_study()