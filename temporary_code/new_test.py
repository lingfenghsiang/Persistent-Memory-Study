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

def plotData(data_path, titles:List[Str], labels:List[Str], fig_name = "tmp.png"):
    df = pd.read_csv(data_path)
    XAxis = df[titles[0]].to_numpy()
    fig, ax1 = plt.subplots(figsize = (8,4.96))
    for i in range(1, len(titles)):
        plt.grid()
        data = df[titles[i]].to_numpy()
        plot0=ax1.plot(XAxis, data, label = labels[i], markersize = 10, linewidth=2)
    ax1.set_xlabel(labels[0], fontsize = 16)
    ax1.set_xscale("log", base=2)
    plt.grid()
    ax1.legend()
    foo_fig = plt.gcf()  # 'get current figure'
    foo_fig.savefig(fig_name,
                    bbox_inches='tight', format='png', dpi=1000, pad_inches=0.0)





def run_code():
    clear_log_cmd = "echo > " + os.path.join(this_file_dir, "..","tmp.log")
    os.system(clear_log_cmd)
    exe_cmd = "numactl -N 0 " + \
        os.path.join(this_file_dir, "..", "build_benchmark", "bin",
                    "microbench") + " -test 1 >> " + os.path.join(this_file_dir, "..","tmp.log")
    os.system(exe_cmd)
    

def run_numa_rap():
    clear_log_cmd = "echo > " + os.path.join(this_file_dir, "..","numa_rap.log")
    os.system(clear_log_cmd)
    exe_cmd = "numactl -N 0 " + \
        os.path.join(this_file_dir, "..", "build_benchmark", "bin",
                    "microbench") + " -test 7 >> " + os.path.join(this_file_dir, "..","numa_rap.log")
    os.system(exe_cmd)

    clear_log_cmd = "echo > " + os.path.join(this_file_dir, "..","numa_rap_dram.log")
    os.system(clear_log_cmd)
    exe_cmd = "numactl -N 0 " + \
        os.path.join(this_file_dir, "..", "build_benchmark", "bin",
                    "microbench") + " -test 7 -nopmm >> " + os.path.join(this_file_dir, "..","numa_rap_dram.log")
    os.system(exe_cmd)

def generate_graph():
    format_log_cmd = python_path + " " + tool_path +\
                " -log_path=" + os.path.join("/home/xlf/Documents/Persistent-Memory-Study", "tmp.log") +\
                " -config_path=" + os.path.join(this_file_dir, "..", "tmp.json") +\
                " -tmp_dir=" + tmp_dir +\
                " -out_dir=" + tmp_dir

    os.system(format_log_cmd)
    plotData(os.path.join(this_file_dir, "..", "tmp", "tmp.prefetching.csv"),
    ["wss", " 1024 imc read ratio", " 1024 pm read ratio"] ,
    ["wss","imc256", "pm256"], 
    os.path.join(this_file_dir, "..", "tmp.png") )

def generate_graph():
    format_log_cmd = python_path + " " + tool_path +\
                " -log_path=" + os.path.join("/home/xlf/Documents/Persistent-Memory-Study", "tmp.log") +\
                " -config_path=" + os.path.join(this_file_dir, "..", "tmp.json") +\
                " -tmp_dir=" + tmp_dir +\
                " -out_dir=" + tmp_dir

    os.system(format_log_cmd)
    plotData(os.path.join(this_file_dir, "..", "tmp", "tmp.prefetching.csv"),
    ["wss", " 1024 imc read ratio", " 1024 pm read ratio"] ,
    ["wss","imc256", "pm256"], 
    os.path.join(this_file_dir, "..", "tmp.png") )

def generate_avoid_prefetch_graph():
    format_log_cmd = python_path + " " + tool_path +\
                " -log_path=" + os.path.join("/home/xlf/Documents/Persistent-Memory-Study", "task8.log") +\
                " -config_path=" + os.path.join(this_file_dir, "..", "tmp.json") +\
                " -tmp_dir=" + tmp_dir +\
                " -out_dir=" + tmp_dir

    os.system(format_log_cmd)
    plotData(os.path.join(this_file_dir, "..", "tmp", "tmp.avd_prefetching.csv"),
    ["wss", " normal load imc read", " normal load pm read", " nt cpy then load imc read", " nt cpy then load pm read"] ,
    ["wss","original imc rd", "original pm rd", "optimized imc rd", "optimized pm rd"], 
    os.path.join(this_file_dir, "..", "tmp_acd_prefetch.png") )


def generate_numa_rap_graph():
    format_log_cmd = python_path + " " + tool_path +\
                " -log_path=" + os.path.join("/home/xlf/Documents/Persistent-Memory-Study", "numa_rap.log") +\
                " -config_path=" + os.path.join(this_file_dir, "..", "tmp.numa_rap.json") +\
                " -tmp_dir=" + tmp_dir +\
                " -out_dir=" + tmp_dir

    os.system(format_log_cmd)

    format_log_cmd = python_path + " " + tool_path +\
                " -log_path=" + os.path.join("/home/xlf/Documents/Persistent-Memory-Study", "numa_rap_dram.log") +\
                " -config_path=" + os.path.join(this_file_dir, "..", "tmp.numa_rap_dram.json") +\
                " -tmp_dir=" + tmp_dir +\
                " -out_dir=" + tmp_dir

    os.system(format_log_cmd)

    plotData(os.path.join(this_file_dir, "..", "tmp", "tmp.numa_rap.csv"),
    ["distance", " 0 across thread latency", " 1 across thread latency"] ,
    ["distance","no_read", "read"], 
    os.path.join(this_file_dir, "..", "tmp.numa_rap_pmm.png") )
    plotData(os.path.join(this_file_dir, "..", "tmp", "tmp.numa_rap_dram.csv"),
    ["distance", " 0 across thread latency", " 1 across thread latency"] ,
    ["distance","no_read", "read"], 
    os.path.join(this_file_dir, "..", "tmp.numa_rap_dram.png") )

# " 256 imc read ratio", " 256 pm read ratio", " 512 imc read ratio", " 512 pm read ratio", " 768 imc read ratio", " 768 pm read ratio", 
#  "imc64", "pm64", "imc128", "pm128", "imc192", "pm192", 

# run_numa_rap()
# send_server("test over", "prefetching test")
# generate_graph()
# generate_avoid_prefetch_graph()

class ExpRunner():
    def __init__(self, log_path: str, numa_node: int = 0, python_path:str = "/home/xlf/anaconda3/bin/python") -> None:
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

    def send_server(self, receiver, text):
        api = "https://sctapi.ftqq.com/SCT118090TOrz56byOfkU24e5Tk784faQ5.send"
        data = {
            'title': receiver,
            'desp': text}
        result = requests.post(api, data=data)
        return(result)

    def run_exp(self, bin_file:str, args: List[str], note: str = "tmp"):
        now = datetime.now()
        time_stamp = now.strftime("%Y-%m-%d-%H:%M:%S")
        log_name = note + "." + time_stamp + ".log"
        if not os.path.isabs(bin_file):
            bin_file = os.path.join(self.project_dir_, bin_file)
        cmd = self.numa_cmd_ + " " + bin_file + " " + " ".join(args) + " > " + os.path.join(self.tmp_dir_,log_name)
        os.system(cmd)
        return os.path.join(self.tmp_dir_,log_name)

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
    
    def plotData(self, data_path, titles:List[Str], labels:List[Str], fig_name = "tmp.png"):
        df = pd.read_csv(data_path)
        XAxis = df[titles[0]].to_numpy()
        fig, ax1 = plt.subplots(figsize = (8,4.96))
        for i in range(1, len(titles)):
            plt.grid()
            data = df[titles[i]].to_numpy()
            plot0=ax1.plot(XAxis, data, label = labels[i], markersize = 10, linewidth=2)
        ax1.set_xlabel(labels[0], fontsize = 16)
        ax1.set_xscale("log", base=2)
        plt.grid()
        ax1.legend()
        foo_fig = plt.gcf()  # 'get current figure'
        foo_fig.savefig(fig_name,
                        bbox_inches='tight', format='png', dpi=1000, pad_inches=0.0)


runner = ExpRunner("tmp", 1)
log_name = runner.run_exp("build_benchmark/bin/microbench",
                     ["-test", "5", "-nopmm"], "read_after_flush")

# log_name = "/home/xlf/Documents/Persistent-Memory-Study/tmp/read_after_flush.2022-02-10-17:46:38.log"


formatted_log_name = runner.format_log(log_name, ["rap.csv",
                                                          "distance",    ["method"]])
runner.plotData("/home/xlf/Documents/Persistent-Memory-Study/tmp/optimize_prefetching.csv",[],[])


# ["wss", " 256 imc read ratio", " 256 pm read ratio", " 512 imc read ratio", " 512 pm read ratio", " 1024 imc read ratio", " 1024 pm read ratio"] ,
# ["wss","rd_ratio_imc_256", "rd_ratio_pm_256","rd_ratio_imc_512", "rd_ratio_pm_512","rd_ratio_imc_1024", "rd_ratio_pm_1024"], 

# runner.plotData("/home/xlf/Documents/Persistent-Memory-Study/tmp/optimize_prefetching.csv",
#                 ["wss",
#                  " normal load latency",
#                 #  " normal load imc read",
#                 #  " normal load pm read",
#                  " nt cpy then load latency",
#                 #  " nt cpy then load imc read",
#                 #  " nt cpy then load pm read"
#                  ],
#                 ["wss",
#                  "original latency",
#                 #  "original imc read",
#                 #  "original pm read",
#                  "optimized latency",
#                 #  "optimized imc read",
#                 #  "optimized pm read"
#                  ],
#                 os.path.join(runner.tmp_dir_, "prefetch.png")
# )
# runner.send_server("test over","test over")
