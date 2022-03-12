import os
import shutil
from sqlite3 import Timestamp
import pandas as pd
import matplotlib.pyplot as plt
from typing import List
from datetime import datetime
import json
import copy


this_file_dir = os.path.abspath(os.path.dirname(__file__))
tmp_dir = os.path.join(this_file_dir, "tmp")
tool_path = os.path.join(this_file_dir,
                         "tools", "format_log.py")
python_path = "/home/xlf/anaconda3/bin/python"

if not os.path.isdir(tmp_dir):
    os.makedirs(tmp_dir)


class ExpRunner():
    def __init__(self, log_path: str, numa_node: int = 0, python_path: str = "/home/xlf/anaconda3/bin/python") -> None:
        this_file_dir = os.path.abspath(os.path.dirname(__file__))
        self.project_dir_ = os.path.join(this_file_dir)
        self.tmp_dir_ = os.path.join(self.project_dir_, "tmp")
        self.python_path_ = python_path
        self.tool_path_ = os.path.join(self.project_dir_,
                                       "tools")
        self.output_base_dir_ = os.path.join(self.project_dir_, "output")
        if numa_node >= 0:
            self.numa_cmd_ = "numactl -N " + str(numa_node)
        else:
            self.numa_cmd_ = ""
        if os.path.isabs(log_path):
            self.log_path_ = log_path
        else:
            self.log_path_ = os.path.join(self.project_dir_, log_path)

    def run_exp(self, bin_file: str, args: List[str], note: str = "tmp"):
        now = datetime.now()
        time_stamp = now.strftime("%Y-%m-%d-%H:%M:%S")
        log_name = note + "." + time_stamp + ".log"
        if not os.path.isabs(bin_file):
            bin_file = os.path.join(self.project_dir_, bin_file)
        cmd = self.numa_cmd_ + " " + bin_file + " " + \
            " ".join(args) + " > " + os.path.join(self.tmp_dir_, log_name)
        os.system(cmd)
        return os.path.join(self.tmp_dir_, log_name)

    def generate_json(self, data, file_name):
        with open(file_name, 'w') as outfile:
            json.dump(data, outfile)

    def format_log(self, log_path, json_config, out_path):
        if not os.path.isabs(log_path):
            log_path = os.path.join(self.tmp_dir_, log_path)
        json_config_path = os.path.join(self.tmp_dir_, "tmp.json")
        self.generate_json(json_config, json_config_path)
        format_log_cmd = self.python_path_ + " " + tool_path +\
            " -log_path=" + log_path +\
            " -config_path=" + json_config_path +\
            " -tmp_dir=" + self.tmp_dir_ +\
            " -out_dir=" + os.path.join(self.output_base_dir_, out_path)
        os.system(format_log_cmd)
        csv_data_name = json_config[0]
        return os.path.join(self.output_base_dir_, out_path, csv_data_name)

    def plotData(self, data_path, titles: List[str], labels: List[str], y_label: str,
                 plot_every = 1, axis_log: bool = True, fig_name="tmp.png"):
        mks = ["o", "v", "^", "<", "*", "x", "p", "s", "D", "P"]
        lineType = ["-", "--", ":", "-."]
        mknum = len(mks)
        linenum = len(lineType)
        df = pd.read_csv(data_path)
        XAxis = df[titles[0]].to_numpy()
        fig, ax1 = plt.subplots(figsize=(8, 4.96))
        for i in range(1, len(titles)):
            plt.grid()
            data = df[titles[i]].to_numpy()
            plot0 = ax1.plot(
                XAxis, data, mks[i % mknum] + lineType[i // linenum],
                markerfacecolor='none',markevery=plot_every, label=labels[i], markersize=10, linewidth=2)
        ax1.set_xlabel(labels[0], fontsize=16)
        ax1.set_ylabel(y_label, fontsize=16)
        if axis_log:
            ax1.set_xscale("log", base=2)
        plt.grid()
        ax1.legend()
        foo_fig = plt.gcf()  # 'get current figure'
        foo_fig.savefig(fig_name,
                        bbox_inches='tight', format='png', dpi=1000, pad_inches=0.0)


class ExpConfig:
    def __init__(self, args=None, note=None, json_config=None, out_dir=None,
                 plot_y_labels=None, x_log = False, plot_every:int = 1, 
                 data_lists: list[list[str]] = None, attach_timestamp: bool = False,
                 plot_labels: list[list[str]] = None, fig_name: list[list[str]] = None) -> None:
        self.args_ = args
        self.note_ = note
        self.json_config_ = json_config
        self.data_lists_ = data_lists
        self.plot_labels_ = plot_labels
        self.fig_name_ = fig_name
        self.out_dir_ = out_dir
        self.y_labels_ = plot_y_labels
        self.x_log_ = x_log
        self.plot_every_ = plot_every
        self.output_attach_timestamp_ = attach_timestamp


exps = {
    "prefetch": ExpConfig(args=["-test", "1"],
                          note="prefetch",
                          json_config=["microbench_prefetching.csv",
                                       "wss",    ["granularity"]],
                          out_dir="micro_bench",
                          x_log=True,
                          data_lists=[
                              ["wss", " 256 imc read ratio", " 256 pm read ratio"]],
                          plot_labels=[
                              ["wss", "rd_ratio_imc_256", "rd_ratio_pm_256"]],
                          plot_y_labels=["read ratio", ],
                          attach_timestamp = True,
                          fig_name=["prefetch.png"]),
    "prefetch_optimize": ExpConfig(
        args=["-test", "8"],
        note="prefetch_optimize",
        json_config=["prefetch_optimize.csv",
                     "wss",    ["type"]],
        out_dir="case_study",
        data_lists=[
            ["wss",
             " normal load imc read ratio",
             " normal load pm read ratio",
             " nt cpy then load imc read ratio",
             " nt cpy then load pm read ratio"
             ]
        ],
        plot_labels=[
            ["wss",
             "original imc read ratio",
             "original pm read ratio",
             "optimized imc read ratio",
             "optimized pm read ratio"
             ]
        ],
        plot_y_labels=["CPU cycles", "read ratio"],
        x_log=True,
        fig_name=[
            "prefetch_optimize_read_size.png"]
    ),
    "rap": ExpConfig(
        args=["-test", "5"],
        note="read_after_persist",
        json_config=["rap.csv",
                     "distance",    ["method", "on dram", "remote"]],
        out_dir="micro_bench",
        data_lists=[
            ["distance",
             " wr_clwb_mfence 0 0 RAP lat",
             " wr_clwb_sfence 0 0 RAP lat",
             " wr_nt_sfence 0 0 RAP lat",
             ],
            ["distance",
             " wr_clwb_mfence 1 0 RAP lat",
             " wr_clwb_sfence 1 0 RAP lat",
             ],
            ["distance",
             " wr_clwb_mfence 0 1 RAP lat",
             " wr_clwb_sfence 0 1 RAP lat",
             " wr_nt_sfence 0 1 RAP lat",
             ],
            ["distance",
             " wr_clwb_mfence 1 1 RAP lat",
             " wr_clwb_sfence 1 1 RAP lat",
             ],
        ],
        plot_labels=[
            ["distance",
             "PM+clwb+mfence",
             "PM+clwb+sfence",
             "PM+nt-store+mfence"
             ],
            [
                "distance",
                "DRAM+clwb+mfence",
                "DRAM+clwb+sfence"
            ],
            ["distance",
             "PM+clwb+mfence",
             "PM+clwb+sfence",
             "PM+nt-store+mfence"
             ],
            [
                "distance",
                "DRAM+clwb+mfence",
                "DRAM+clwb+sfence"
            ]
        ],
        plot_every= 4,
        plot_y_labels=["CPU cycles", "CPU cycles", "CPU cycles", "CPU cycles",],
        fig_name=[
            "read_after_flush_pm_local.png",
            "read_after_flush_dram_local.png",
            "read_after_flush_pm_remote.png",
            "read_after_flush_dram_remote.png",
            ]      
    ),
    "rd_throughput_prefetch": ExpConfig(
        args=["-test", "9"],
        note="read_throuput_against_prefetch",
        json_config=["read_throuput_against_prefetch.csv",
                     "thread num",    ["type"]],
        out_dir="case_study",
        data_lists=[
            ["thread num",
             " normal load perceived throughput",
             " nt cpy then load perceived throughput",
             ],
            ["thread num",
             " normal load latency",
             " nt cpy then load latency",
             ]
        ],
        plot_labels=[
            ["thread num",
             "normal perceived throughput",
             "optimized perceived throughput",
             ],
            ["thread num",
             "normal latency",
             "optimized latency",
             ]
        ],
        plot_y_labels=["MB/s", "CPU cycles"],
        fig_name=["prefetch_multithread_rd_throughput.png",
                  "prefetch_multithread_rd_lat.png"]
    )
}


def general_task_runner(config: ExpConfig):

    runner = ExpRunner("tmp", 0)
    now = datetime.now()
    time_stamp = time_stamp = now.strftime("%Y-%m-%d-%H:%M:%S")
    config = copy.deepcopy(config)
    if config.output_attach_timestamp_:
        config.json_config_[0] = config.json_config_[0][:-4] + '.' + time_stamp + config.json_config_[0][-4:]
        for i in range(len(config.fig_name_)):
            config.fig_name_[i] = config.fig_name_[i][:-4] + '.' + time_stamp + config.fig_name_[i][-4:]

    log_name = runner.run_exp("build_benchmark/bin/microbench",
                            config.args_, config.note_)

    formatted_log_name = runner.format_log(
        log_name, config.json_config_, config.out_dir_)

    size = len(config.plot_labels_)
    for i in range(size):
        runner.plotData(data_path=formatted_log_name,
                        titles=config.data_lists_[i],
                        labels=config.plot_labels_[i],
                        axis_log=config.x_log_,
                        plot_every= config.plot_every_,
                        y_label=config.y_labels_[i],
                        fig_name=os.path.join(runner.output_base_dir_,
                                              config.out_dir_, config.fig_name_[i])
                        )


def prepare_output_dir():
    output_dir = os.path.join(this_file_dir, "output")
    if not os.path.isdir(output_dir):
        os.makedirs(output_dir)
    case_study_dir = os.path.join(output_dir, "case_study")
    if not os.path.isdir(case_study_dir):
        os.makedirs(case_study_dir)
    micro_bench_dir = os.path.join(output_dir, "micro_bench")
    if not os.path.isdir(micro_bench_dir):
        os.makedirs(micro_bench_dir)

# compile code for case study, download YCSB and generate workload files
def prepare_case_study():
    src_dir = os.path.join(this_file_dir, "case_study")
    build_dir = os.path.join(this_file_dir, "build_cases")
    if not os.path.isdir(build_dir):
        os.makedirs(build_dir)
    build_cmd = "cd " + build_dir + " && cmake " + src_dir + " && make -j"
    os.system(build_cmd)

    if not os.path.exists(os.path.join(tmp_dir, "ycsb-0.17.0.tar.gz")):
        ycsb_download_cmd = "cd " + tmp_dir + \
            " && wget https://github.com/brianfrankcooper/YCSB/releases/download/0.17.0/ycsb-0.17.0.tar.gz"
        os.system(ycsb_download_cmd)
    if not os.path.exists(os.path.join(tmp_dir, "ycsb-0.17.0")):
        os.system("cd " + tmp_dir + " && tar -xvf " +
                  os.path.join(tmp_dir, "ycsb-0.17.0.tar.gz"))

    prepare_workload_cmd = python_path + " " + \
        os.path.join(this_file_dir, "tools",
                     "generate_workload.py") + " -op_num=12000000"
    print(prepare_workload_cmd)
    os.system(prepare_workload_cmd)

# run FAST&FAIR, CCEH


def run_case_study(max_worker, pmem_directory):
    build_dir = os.path.join(this_file_dir, "build_cases")
    # fastfair test

    def fast_fair_test():
        for i in ["fastfair_original_test", "fastfair_rap_mod_test"]:
            init_command = "echo >" + os.path.join(tmp_dir,  i + ".log")
            os.system(init_command)
            for j in range(1, max_worker + 1):
                run_cmd = "numactl -N 0 " + os.path.join(build_dir, i) + " -thread " + str(
                    j) + " >> " + os.path.join(tmp_dir,  i + ".log")
                os.system(run_cmd)
    fast_fair_test()

    # cceh test
    def cceh_test(on_pm: bool = True):
        if on_pm:
            pmem_dir = pmem_directory
            media_type = "pmm"
        else:
            pmem_dir = "/dev/shm/"
            media_type = "dram"
        init_command = "echo >" + \
            os.path.join(tmp_dir,  "cceh_" + media_type + ".log")
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

    general_task_runner(exps["prefetch_optimize"])
    general_task_runner(exps["rd_throughput_prefetch"])

# compile code for microbenchmarks


def prepare_microbench():
    src_dir = os.path.join(this_file_dir, "micro_benchmarks")
    build_dir = os.path.join(this_file_dir, "build_benchmark")
    if not os.path.isdir(build_dir):
        os.makedirs(build_dir)
    build_cmd = "cd " + build_dir + " && cmake " + src_dir + " && make -j"
    os.system(build_cmd)

def run_microbench_except_prefetching():
    build_dir = os.path.join(this_file_dir, "build_benchmark", "bin")
    # read buffer amplification test
    init_command = "echo >" + os.path.join(tmp_dir,  "task0.log")
    os.system(init_command)
    rd_amp_run_cmd = "numactl -N 0 " + \
        os.path.join(build_dir, "microbench") + " -test 0 >> " + \
        os.path.join(tmp_dir,  "task0.log")
    os.system(rd_amp_run_cmd)
    # write buffer test
    init_command = "echo >" + os.path.join(tmp_dir,  "task2.log")
    os.system(init_command)
    wr_buf_run_cmd = "numactl -N 0 " + \
        os.path.join(build_dir, "microbench") + " -test 2 >> " + \
        os.path.join(tmp_dir,  "task2.log")
    os.system(wr_buf_run_cmd)
    # test the seperate read and write buffer
    init_command = "echo >" + os.path.join(tmp_dir,  "task4.log")
    os.system(init_command)
    seperate_rd_wr_buf_cmd = "numactl -N 0 " + \
        os.path.join(build_dir, "microbench") + " -test 4 >> " + \
        os.path.join(tmp_dir,  "task4.log")
    os.system(seperate_rd_wr_buf_cmd)
    # test the read after persist behavior, pm version
    general_task_runner(exps["rap"])
    
    # read after persist, dram version
    init_command = "echo >" + os.path.join(tmp_dir,  "task5_dram.log")
    os.system(init_command)
    rap_cmd = "numactl -N 0 " + os.path.join(build_dir, "microbench") + \
        " -nopmm -test 5 >> " + os.path.join(tmp_dir,  "task5_dram.log")
    os.system(rap_cmd)
    # test latency on persistent memory
    init_command = "echo >" + os.path.join(tmp_dir,  "task6.log")
    os.system(init_command)
    access_lat_cmd = "numactl -N 0 " + \
        os.path.join(build_dir, "microbench") + " -test 6 >> " + \
        os.path.join(tmp_dir,  "task6.log")
    print(access_lat_cmd)
    os.system(access_lat_cmd)


def run_microbench_prefetching():
    general_task_runner(exps["prefetch"])


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
              " -log_path=" + os.path.join(tmp_dir, "task2.log") +
              " -config_path=" + os.path.join(this_file_dir, "tools", "microbench_config2.json") +
              " -tmp_dir=" + tmp_dir +
              " -out_dir=" + micro_bench_dir)

    # task4 to test seperate buffer does not need a graph

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
    shutil.copy(os.path.join(tmp_dir, "task2.log"), os.path.join(
        this_file_dir, "output", "micro_bench", "seperate_rd_wr_buf.log"))
    run_plot_script("plot_bench_lat.py")
    # run_plot_script("plot_bench_prefetching.py")
    run_plot_script("plot_bench_rd_amp.py")

    run_plot_script("plot_bench_wr_buf.py")
    run_plot_script("plot_case_btree.py")
    run_plot_script("plot_case_cceh.py")

prepare_output_dir()
prepare_microbench()
run_microbench_except_prefetching()
run_microbench_prefetching()
prepare_case_study()
run_case_study(8, "/mnt/pmem/")
format_logs()
plot_results()
