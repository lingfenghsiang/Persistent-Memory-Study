# Persistent memory study
Implementation of the paper:

Lingfeng Xiang, Xingsheng Zhao, Jia Rao, Song Jiang and Hong Jiang, **“Characterizing the Performance of Intel Optane Persistent Memory -- A Close Look at its On-DIMM Buffering,”** accepted to appear in Proceedings of the 2022 EuroSys Conference, April 26-29, 2021, Rennes, France

This repository contains the code for microbenchmarks and case studies. It's recommended to execute the "run.py" script, because it
compiles the code and generate worloads for case studies. Please see [Usage](#usage).
## Table of Contents

<!-- - [Background](#background) -->
- [Prerequisites](#prerequisites)
- [Usage](#usage)
	- [Before Run](#before-run)
	- [Click and Run](#click-and-run)
  - [Reproduce results from the paper](#reproduce-results-from-the-paper)
  - [Change working example size for case study](#change-working-example-size-for-case-study)
  - [Details](#details)
- [Matching Paper Results](#matching-paper-results)
- [Miscellaneous](#Miscellaneous)
<!-- ## Background -->

## Prerequisites

### Hardware requirements
Your machine needs to have Intel Optane DC PMM installed.
Besides, your machine also needs AVX512, CLWB, CLFLUSH and CLFLUSHOPT instruction sets.

It's encouraged to have at least 16GB space on the volatile file system `/dev/shm`, 
because the case study uses 16GB by default on to test CCEH performance. 
If you want to use less space, you could change that in file `micro_benchmarks/compiling_config.cmake`, or
you could also change that when you execute the program.
To check the size of your volatile file system you could use:
```
df -h | grep /dev/shm
```

Finally, it's also encouraged to install your Intel Optane DC PMM on NUMA node 0, if you have more than one socket, 
because the benchmark and case studies runs on node 0 by default.
To check where the Intel Optane DC PMM mounts, you may use
```
ndctl list -v
```
You could get hints that show. If the node is not 0, please put the PM DIMM on other sockets and try again.
```
[
  {
    "dev":"namespace1.0",
    "mode":"fsdax",
    "map":"dev",
    "size":133175443456,
    "uuid":"f1aa8979-d552-4097-adb9-ebdf822ef5d7",
    "raw_uuid":"74c70555-8270-45d8-a76f-5ddc1e2b8838",
    "sector_size":512,
    "align":2097152,
    "blockdev":"pmem1",
    "numa_node":0
  }
]
```

### Software requirements

We run the code on Ubuntu 20.04 LTS, and the compatibility on other Linux distros are not verified. 
To run the code you need to install these packages:

```
sudo apt install libvmem-dev libpmemobj-dev libssl-dev libgflags-dev libssl-dev numactl cmake openjdk-11-jdk
```
To run our "click-and-run" script, you need to have python and the some package including `pandas`, `matplotlib` and `numpy`.
To set up the environment, it's encouraged to use [`conda`](https://docs.anaconda.com/anaconda/install/linux/#installing-on-linux).

You may install the package via:
```
conda install pandas matplotlib numpy
```

The Intel Optane DC PMM should be configured as non-interleaved for microbenchmarks and interleaved for case studies.

#### Set PM in non-interleaved mode
All the Intel Optane DC PMM devices must be unmounted before namespaces are destroyed.
```
# destroy current namespaces on Intel Optane DC PMM
ndctl destroy-namespace -f all
# reboot is required after this
ipmctl create -goal PersistentMemoryType=AppDirectNotInterleaved
# execute this after reboot
ndctl create-namespace
```
#### Set PM in interleaved mode
All the Intel Optane DC PMM devices must be unmounted before namespaces are destroyed.
```
# destroy current namespaces on Intel Optane DC PMM
ndctl destroy-namespace -f all
# reboot is required after this
ipmctl create -goal
# execute this after reboot
ndctl create-namespace
```
For more details, please refer https://docs.pmem.io/persistent-memory/getting-started-guide.

## Usage
### Before run
Before you run, something must be set up. You need to
1. Specify where the Intel Optane DC PMM pool locates for microbenchmarks.
2. Specify where the Intel Optane DC PMM pool locates for the case study.
3. Specify the location of your python with packages installed.
4. Set CPU in "performance" mode.
5. Download the code of this repository.
6. Initialize git submodules.

#### Specify microbenchmark memory pool 
If your PM device is mounted at "/mnt/pmem".
The default pool path for microbenchmarks is "/mnt/pmem/bench_map_file", specified in file 
`micro_benchmarks/compiling_config.cmake`.

#### Specify case study memory pool 
If your PM device is mounted at "/mnt/pmem".
The default pool path arguement for case study is "/mnt/pmem/", specified in file 
`case_study/path.cmake`.

#### Specify python path
If your user name is `foo`, and your conda environment is installed at `/home/foo/anaconda3/bin/python`,
you need to specify this in file [run.py:7](run.py#L7) at the beginning as
```
python_path = "/home/foo/anaconda3/bin/python"
```
#### Set CPU in performance mode
Run the command below as root user.
```
echo "performance" | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
```
#### Initialize git submodules
Run the command below in the repository folder.
```
git submodule init
git submodule update
```

### Click and Run
Run the code by
```
python3 run.py
```
You should run the command as root user, bacause getting the DIMM information requires that.
When the execution is over, data and graphs will be generated in folder [output](output).

### Reproduce results from the paper
Simply run the script is not enough. Some tests needs the machine to be reconfigured. These important steps are [run.py:217](run.py#L217) to [run.py:223](run.py#L223), as enclosed below:
```
217 prepare_microbench()                  #step 1
218 run_microbench_except_prefetching()   #step 1
219 run_microbench_prefetching()          #step 2
220 prepare_case_study()                  #step 3
221 run_case_study(6, "/mnt/pmem/")       #step 3
222 format_logs()                         #step 4
223 plot_results()                        #step 4
```
You need to follow the steps in correct order:
> Step 1.
>> Set persistent memory in non-interleaved mode. [HOWTO](#set-pm-in-non-interleaved-mode)
>
>> Set CPU in performance mode. [HOWTO](#set-cpu-in-performance-mode)
>
>> Mount DCPMM device at directory "/mnt/pmem" in dax mode. [HOWTO](#how-do-i-mount-my-pm-device)
>
>> Comment all steps except step 1 (line 217, 218)
>
>> Run "python3 run.py".


> Step 2.
>> Reboot the machine
>
>> Turn off the CPU prefetching in BIOS, including hardware prefetching, adjacent cacheline prefetching and LLC (last level cache) prefetching
(if available)
>
>> Set CPU in performance mode. [HOWTO](#set-cpu-in-performance-mode)
>
>> Mount DCPMM device at directory "/mnt/pmem" in dax mode. [HOWTO](#how-do-i-mount-my-pm-device)
>
>> Comment all steps except step 2 (line 219)
>
>> Run "python3

> Step 3. The integer argument in "run_case_study" is the max number of
working threads and the string arguement is the folder path of persistent memory pool, which must end with "/".
>> Restore BIOS configuration in Step 1
>
>> Set the persistent memory in interleaved mode. [HOWTO](#set-cpu-in-performance-mode)
>
>> Set CPU in performance mode. [HOWTO](#set-pm-in-interleaved-mode)
>
>> Mount DCPMM device at directory "/mnt/pmem" in dax mode. [HOWTO](#how-do-i-mount-my-pm-device)
>
>> Comment all steps except step 3 (line 220, 221)
>
>> Run "python3 run.py".


> Step 4.
>> Comment all steps except step 4 (line 222, 223)
>
>> Run "python3 run.py"



### Change working example size for case study
To run on different size for case study, please change -op_num=12000000 at [run.py:26](run.py#L26) to the size you like.
### Details
For more details about micro-benchmarks, please go to folder [micro_benchmarks](micro_benchmarks).

For more details about micro-benchmarks, please go to folder [case_study](case_study).

## Matching Paper Results
Figure 2. Read amplification on a DIMM.
|  Figure info   | Contents  |
|  ----  | ----  |
| Raw log path  | `tmp/task0.log`  |
| Formatted data path  | `output/micro_bench/microbench_rd_amp.csv`  |
| Graph path  | `output/micro_bench/read_amp.png`  |
| Plot script path  | `tools/plot_bench_rd_amp.py`  |

Figure 3. On-DIMM prefetching
|  Figure info   | Contents  |
|  ----  | ----  |
| Raw log path  | `tmp/task1.log`  |
| Formatted data path  | `output/micro_bench/microbench_trigger_prefetching.csv`  |
| Graph path  |  `output/micro_bench/prefetching.png` |
| Plot script path  | `tools/plot_bench_prefetching.py` |

Figure 4. Write amplification
|  Figure info   | Contents  |
|  ----  | ----  |
| Raw log path  | `tmp/task2.log`  |
| Formatted data path  | `output/micro_bench/microbench_wr_buf.csv`  |
| Graph path  |  `output/micro_bench/write_buf.png` |
| Plot script path  | `tools/plot_bench_wr_buf.py` |

Figure 5. Write buffer hit ratio. This generated figure only shows the write buffer hit ratio on current machine. To evaluate the difference between G1 and G2 Optane, figures across different machines need to be collected.
|  Figure info   | Contents  |
|  ----  | ----  |
| Raw log path  |  `tmp/task2.log`  |
| Formatted data path  | `output/micro_bench/microbench_wr_buf.csv`   |
| Graph path  | `output/micro_bench/write_buf_hit_ratio.png`  |
| Plot script path  | `tools/plot_bench_wr_buf.py` |

Figure 7. Read after persist latency
|  Figure info   | Contents  |
|  ----  | ----  |
| Raw log path  | `tmp/task5.log` and `tmp/task5_dram.log`    |
| Formatted data path  | `output/micro_bench/microbench_read_after_persist.csv` and `output/micro_bench/microbench_read_after_persist_dram.csv`  |
| Graph path  |  `output/micro_bench/read_after_persist.png` |
| Plot script path  | `tools/plot_bench_read_after_persist.py` |

Figure 8. Latency of different write models long with read.
|  Figure info   | Contents  |
|  ----  | ----  |
| Raw log path  | `tmp/task6.log`  |
| Formatted data path  | `output/micro_bench/microbench_lat.csv`  |
| Graph path  |  `output/micro_bench/lat.png` |
| Plot script path  | `tools/plot_bench_lat.py` |

Figure 10. CCEH case study
|  Figure info   | Contents  |
|  ----  | ----  |
| Raw log path  | `tmp/cceh_dram.log`, `tmp/cceh_pmm.log`, `tmp/cceh_preread_dram.log` and  `tmp/cceh_preread_pmm.log` |
| Formatted data path  | `output/case_study/cceh_original_dram.csv`, `output/case_study/cceh_original_pmm.csv`, `output/case_study/cceh_with_preread_dram.csv` and `output/case_study/cceh_with_preread_pmm.csv` |
| Graph path  | `output/case_study/cceh.png`  |
| Plot script path  | `tools/plot_case_cceh.py`  |

Figure 12. FAST & FAIR case study. This generated graph only shows the results on current machine. To compare the results on G1 and G2 Optane, graphs across different machines needs to be collected.
|  Figure info   | Contents  |
|  ----  | ----  |
| Raw log path  | `tmp/fastfair_original_test.log` and `tmp/fastfair_rap_mod_test.log` |
| Formatted data path  |  `output/case_study/fastfair_original_test.csv` and `output/case_study/fastfair_rap_mod_test.csv`|
| Graph path  |  `output/case_study/fastfair.png` |
| Plot script path  | `tools/plot_case_btree.py` |

## Miscellaneous
### What if I forget to install some dependencies?
 If you forget to install any dependencies before running "run.py", you need to delete the "tmp" folder. Once the dependencies are fixed, the "run.py" will create "tmp" folder again.
### How do I mount my PM device?
Once you have created the namespaces of PM devices, you need to format the device in file sytems that support direct access (e.g. EXT4). You may have a device named "/dev/pmem0". Please execute
```
mkfs.ext4 /dev/pmem0
# mount the device at /mnt/pmem
mount -o dax /dev/pmem0 /mnt/pmem
```

### How to turn off my CPU prefetching?
To run the prefetching test, you have to turn off the CPU prefetching,CPU prefetching, including hardware prefetching, adjacent cacheline prefetching, LLC prefetch, if there are any. Normally, the CPU prefetching configuration is included in the computer BIOS system and could be found in the advanced CPU configuration. Detailed operations varies on machines from different vendors.

### My ipmctl tool does not work.
The offical ipmctl may not apply to 2nd generation Intel Optane DC PMM. If your ipmctl cannot configure the PM device, you need to download and compile the latest code of ipmctl. For more details please refer https://docs.pmem.io/ipmctl-user-guide/installing-ipmctl/building-and-installing-ipmctl-from-source-on-linux
