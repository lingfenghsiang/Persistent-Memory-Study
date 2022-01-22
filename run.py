import os
import shutil

this_file_dir = os.path.abspath(os.path.dirname(__file__))

def prepare_case_study():
    src_dir = os.path.join(this_file_dir, "case_study")
    build_dir = os.path.join(this_file_dir, "build_cases")
    if os.path.isdir(build_dir):
        shutil.rmtree(build_dir)
    os.makedirs(build_dir)
    build_cmd = "cd " + build_dir + " && cmake " + src_dir + " && make -j"
    os.system(build_cmd)
    if not os.path.exists(os.path.join(this_file_dir, "ycsb-0.17.0.tar.gz")):
        ycsb_download_cmd = "wget https://github.com/brianfrankcooper/YCSB/releases/download/0.17.0/ycsb-0.17.0.tar.gz"
        os.system(ycsb_download_cmd)
    if not os.path.exists(os.path.join(this_file_dir, "ycsb-0.17.0")):
        os.system("tar -xvf " + os.path.join(this_file_dir, "ycsb-0.17.0.tar.gz"))

prepare_case_study()