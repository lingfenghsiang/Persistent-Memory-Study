from ast import Str
import os
import pandas as pd
import matplotlib.pyplot as plt
from typing import List
import requests
from datetime import datetime
import json

this_file_dir = os.path.abspath(os.path.dirname(__file__))
tmp_dir = os.path.join(this_file_dir, "..", "tmp")
python_path = "/home/xlf/anaconda3/bin/python"
tool_path = os.path.join(this_file_dir, "..",
                         "tools", "format_log.py")

class ExpRunner():
    def __init__(self, log_path: str, numa_node: int = 0, python_path: str = "/home/xlf/anaconda3/bin/python") -> None:
        this_file_dir = os.path.abspath(os.path.dirname(__file__))
        self.project_dir_ = os.path.join(this_file_dir, "..")
        self.tmp_dir_ = os.path.join(self.project_dir_, "tmp")
        self.python_path_ = python_path
        self.tool_path_ = os.path.join(self.project_dir_,
                                       "tools")
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

    def format_log(self, log_path, json_config):
        if not os.path.isabs(log_path):
            log_path = os.path.join(self.tmp_dir_, log_path)
        json_config_path = os.path.join(self.tmp_dir_, "tmp.json")
        self.generate_json(json_config, json_config_path)
        format_log_cmd = self.python_path_ + " " + tool_path +\
            " -log_path=" + log_path +\
            " -config_path=" + json_config_path +\
            " -tmp_dir=" + self.tmp_dir_ +\
            " -out_dir=" + self.tmp_dir_
        os.system(format_log_cmd)
        csv_data_name = json_config[0]
        return os.path.join(self.tmp_dir_, csv_data_name)

    def plotData(self, data_path, titles: List[Str], labels: List[Str], axis_mode:bool = True, fig_name="tmp.png"):
        df = pd.read_csv(data_path)
        XAxis = df[titles[0]].to_numpy()
        fig, ax1 = plt.subplots(figsize=(8, 4.96))
        for i in range(1, len(titles)):
            plt.grid()
            data = df[titles[i]].to_numpy()
            plot0 = ax1.plot(
                XAxis, data, label=labels[i], markersize=10, linewidth=2)
        ax1.set_xlabel(labels[0], fontsize=16)
        if axis_mode:
            ax1.set_xscale("log", base=2)
        plt.grid()
        ax1.legend()
        foo_fig = plt.gcf()  # 'get current figure'
        foo_fig.savefig(fig_name,
                        bbox_inches='tight', format='png', dpi=1000, pad_inches=0.0)


class ExpConfig:
    def __init__(self, args=None, note=None, json_config=None, data_lists=None, plot_labels=None, fig_name=None) -> None:
        self.args_ = args
        self.note_ = note
        self.json_config_ = json_config
        self.data_lists_ = data_lists
        self.plot_labels_ = plot_labels
        self.fig_name_ = fig_name


exps = {
    "prefetch": ExpConfig(args=["-test", "1"],
                          note="prefetch_dcu_streamer",
                          json_config=["microbench_trigger_prefetching.csv",
                                       "wss",    ["granularity"]],
                          data_lists=["wss", " 256 imc read ratio", " 256 pm read ratio"],
                          plot_labels=["wss", "rd_ratio_imc_256", "rd_ratio_pm_256"],
                          fig_name="prefetch.png"),
    "prefetch_optimize": ExpConfig(
        args=["-test", "8"],
        note="prefetch_optimize",
        json_config=["prefetch_optimize.csv",
                     "wss",    ["type"]],
        data_lists=["wss",
                    " normal load latency",
                    " nt cpy then load latency",

                    # " normal load imc read",
                    # " normal load pm read",
                    # " nt cpy then load imc read",
                    # " nt cpy then load pm read"
                    ],
        plot_labels=["wss",
                      "original latency",
                      "optimized latency",

                    #  "original imc read",
                    #  "original pm read",
                    #  "optimized imc read",
                    #  "optimized pm read"
                     ],
        fig_name="prefetch_optimize.png"
    ),
    "rap": ExpConfig(
        args= ["-test", "5"],
        note="read_after_persist",
        json_config=["rap.csv",
                     "distance",    ["method", "on dram", "remote"]]
    ),
    "rd_throughput_prefetch": ExpConfig(
        args=["-test", "9"],
        note="read_throuput_against_prefetch",
        json_config=["read_throuput_against_prefetch.csv",
                     "thread num",    ["type"]],
        data_lists=["thread num",
                    # " normal load latency",
                    # " nt cpy then load latency",
                    " normal load pm throughput",
                    " normal load perceived throughput",
                    " nt cpy then load pm throughput",
                    " nt cpy then load perceived throughput",
                    ],
        plot_labels=["thread num",
                    #  "normal latency",
                    #  "optimized latency",
                     "normal pm throughput",
                     "normal perceived throughput",
                     "optimized pm throughput",
                     "optimized perceived throughput",
                     ],
        fig_name="prefetch_multithread_rd.png"
    )
}


runner = ExpRunner("tmp", 0)
exp_candidate = exps["prefetch"]

log_name = runner.run_exp("build_benchmark/bin/microbench",
                          exp_candidate.args_, exp_candidate.note_)

formatted_log_name = runner.format_log(log_name, exp_candidate.json_config_)


runner.plotData(data_path = formatted_log_name,
                titles = exp_candidate.data_lists_,
                labels = exp_candidate.plot_labels_,
                axis_mode=True,
                fig_name = os.path.join(runner.tmp_dir_, exp_candidate.fig_name_)
                )
