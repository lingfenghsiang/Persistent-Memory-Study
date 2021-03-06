# Persistent memory study
Implementation of the paper:

Lingfeng Xiang, Xingsheng Zhao, Jia Rao, Song Jiang and Hong Jiang, [**“Characterizing the Performance of Intel Optane Persistent Memory -- A Close Look at its On-DIMM Buffering,”**](https://dl.acm.org/doi/10.1145/3492321.3519556) in Proceedings of the EuroSys Conference, April 5-8, 2022, Rennes, France

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
- [Notion](#notion)
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
You could get hints shown below. If the node is not 0, please put the PM DIMM on other sockets and try again.
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
To run our "click-and-run" script, you need to have at least python3.9 and the some package including `pandas`, `matplotlib` and `numpy`.
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
you need to specify this in file [run.py:16](run.py#L16) at the beginning as
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
/home/your_name/anaconda/bin/python3 run.py
```
You should run the command as root user, bacause getting the DIMM information requires that.
When the execution is over, data and graphs will be generated in folder [output](output).

### Reproduce results from the paper
Simply run the script is not enough. Some tests needs the machine to be reconfigured. These important steps are [run.py:497](run.py#L497) to [run.py:504](run.py#L504), as enclosed below:
```
497 prepare_output_dir()                  #step 1
498 prepare_microbench()                  #step 1
499 run_microbench_except_prefetching()   #step 1
500 run_microbench_prefetching()          #step 2
501 prepare_case_study()                  #step 3
502 run_case_study(6, "/mnt/pmem/")       #step 3
503 format_logs()                         #step 4
504 plot_results()                        #step 4
```
You need to follow the steps in correct order:
> Step 1. 90 minutes and 16GB space on Intel Optane DC PMM.
>> Set persistent memory in non-interleaved mode. [HOWTO](#set-pm-in-non-interleaved-mode)
>
>> Set CPU in performance mode. [HOWTO](#set-cpu-in-performance-mode)
>
>> Mount DCPMM device at directory "/mnt/pmem" in dax mode. [HOWTO](#how-do-i-mount-my-pm-device)
>
>> Comment all steps except step 1 (line 497-499)
>
>> Run "/home/your_name/anaconda/bin/python3 run.py".


> Step 2. This step may test multiple prefetchers and each round takes 15 minutes and 16GB space on Intel Optane DC PMM. 
>> Reboot the machine
>
>> Turn off the CPU prefetcher in BIOS (hardware prefetching/adjacent cacheline prefetching/DCU streamer prefetching)
>
>> Set CPU in performance mode. [HOWTO](#set-cpu-in-performance-mode)
>
>> Mount DCPMM device at directory "/mnt/pmem" in dax mode. [HOWTO](#how-do-i-mount-my-pm-device)
>
>> Comment all steps except step 2 (line 500)
>
>> Run "/home/your_name/anaconda/bin/python3 run.py".
>
>> Repeat step 2 and test a different prefetcher.

> Step 3. The integer argument in "run_case_study" is the max number of
working threads and the string arguement is the folder path of persistent memory pool, which must end with "/". 40 minutes and 32GB space on DRAM and 16GB on Intel Optane DC PMM.
>> Restore BIOS configuration in Step 1
>
>> Set the persistent memory in interleaved mode. [HOWTO](#set-cpu-in-performance-mode)
>
>> Set CPU in performance mode. [HOWTO](#set-pm-in-interleaved-mode)
>
>> Mount DCPMM device at directory "/mnt/pmem" in dax mode. [HOWTO](#how-do-i-mount-my-pm-device)
>
>> Comment all steps except step 3 (line 501, 502)
>
>> Run "/home/your_name/anaconda/bin/python3 run.py".


> Step 4.
>> Comment all steps except step 4 (line 503, 504)
>
>> Run "/home/your_name/anaconda/bin/python3 run.py"



### Change working example size for case study
To run on different size for case study, please change -op_num=12000000 at [run.py:304](run.py#L304) to the size you like.
### Details
For more details about micro-benchmarks (How to run, how is the code organized), please go to folder [micro_benchmarks](micro_benchmarks).

For more details about micro-benchmarks (How to run, how is the code organized), please go to folder [case_study](case_study).

## Matching Paper Results
### Figure 2. Read amplification on a DIMM.



|  Figure info   | Contents  |
|  ----  | ----  |
| Raw log path  | `tmp/task0.log`  |
| Formatted data path  | `output/micro_bench/microbench_rd_amp.csv`  |
| Graph path  | `output/micro_bench/read_amp.png`  |
| Plot script path  | `tools/plot_bench_rd_amp.py`  |

The command to run a solitary test:
```
numactl -N 0 build_benchmark/bin/microbench  -test 0 > name_of_the_log.log
```

### Figure 3. On-DIMM prefetching
Each time step 2 runs, a new formatted log and graph is generated.
Since multiple prefetchers need to be tested and prefethcers reset in BIOS, file names are tagged with a timestamp. The format is "year-month-day-hour:minute:second".

|  Figure info   | Contents  |
|  ----  | ----  |
| Raw log path  | `tmp/prefetch.YYYY-MM-DD-HH:MM:SS.log`  |
| Formatted data path  | `output/micro_bench/microbench_prefetching.YYYY-MM-DD-HH:MM:SS.csv`  |
| Graph path  | `output/micro_bench/prefetch.YYYY-MM-DD-HH:MM:SS.png`  |
| Plot script path  | function `general_task_runner()` in `run.py`  |

The command to run a solitary test:
```
numactl -N 0 build_benchmark/bin/microbench  -test 0 > name_of_the_log.log
```

### Figure 4. Write amplification
|  Figure info   | Contents  |
|  ----  | ----  |
| Raw log path  | `tmp/task2.log`  |
| Formatted data path  | `output/micro_bench/microbench_wr_buf.csv`  |
| Graph path  |  `output/micro_bench/write_buf.png` |
| Plot script path  | `tools/plot_bench_wr_buf.py` |

The command to run a solitary test:
```
numactl -N 0 build_benchmark/bin/microbench  -test 0 > name_of_the_log.log
```
### Figure 5. Write buffer hit ratio.

This generated figure only shows the write buffer hit ratio on current machine. To evaluate the difference between G1 and G2 Optane, figures across different machines need to be collected.
|  Figure info   | Contents  |
|  ----  | ----  |
| Raw log path  |  `tmp/task2.log`  |
| Formatted data path  | `output/micro_bench/microbench_wr_buf.csv`   |
| Graph path  | `output/micro_bench/write_buf_hit_ratio.png`  |
| Plot script path  | `tools/plot_bench_wr_buf.py` |

This test is executed in test for Figure 4.

### Figure 7. Read after persist latency
The raw data log has a timestamp in the format of "year-month-day-hour:minute:second".

|  Figure info   | Contents  |
|  ----  | ----  |
| Raw log path  | `tmp/read_after_persist.YYYY-MM-DD-HH:MM:SS.log`    |
| Formatted data path  | `output/micro_bench/rap.csv`   |
| Graph path  | `output/micro_bench/read_after_flush_dram_local.png`, `output/micro_bench/read_after_flush_dram_remote.png`, `output/micro_bench/read_after_flush_pm_local.png` and `output/micro_bench/read_after_flush_pm_remote.png` |
| Plot script path  | function `general_task_runner()` in `run.py` |

The command to run a solitary test:
On PM
```
numactl -N 0 build_benchmark/bin/microbench  -test 5 > name_of_the_log.log
```
On DRAM
```
numactl -N 0 build_benchmark/bin/microbench  -nopmm -test 5 > name_of_the_log.log
```

### Figure 8. Latency of different write models along with read.
|  Figure info   | Contents  |
|  ----  | ----  |
| Raw log path  | `tmp/task6.log`  |
| Formatted data path  | `output/micro_bench/microbench_lat.csv`  |
| Graph path  |  `output/micro_bench/lat.png` |
| Plot script path  | `tools/plot_bench_lat.py` |

The command to run a solitary test:
 ```
 numactl -N 0 build_benchmark/bin/microbench  -test 6 > name_of_the_log.log
 ```

### Figure 10. CCEH case study
|  Figure info   | Contents  |
|  ----  | ----  |
| Raw log path  | `tmp/cceh_dram.log`, `tmp/cceh_pmm.log`, `tmp/cceh_preread_dram.log` and  `tmp/cceh_preread_pmm.log` |
| Formatted data path  | `output/case_study/cceh_original_dram.csv`, `output/case_study/cceh_original_pmm.csv`, `output/case_study/cceh_with_preread_dram.csv` and `output/case_study/cceh_with_preread_pmm.csv` |
| Graph path  | `output/case_study/cceh.png`  |
| Plot script path  | `tools/plot_case_cceh.py`  |

The command to run a solitary test:

No prefetch thread: thread_num is the number of threads
```
 numactl -N 0 build_cases/cceh_test -pool_dir /mnt/pmem -thread thread_num > name_of_the_log.log 
```
With prefetch thread: thread_num is the number of working threads, if thread_num is 10, then 10 prefetch thread and 10 working threads
```
numactl -N 0 build_cases/cceh_test -pool_dir /mnt/pmem  -preread  -thread thread_num > name_of_the_log.log
```
### Figure 12. FAST & FAIR case study.
This generated graph only shows the results on current machine. To compare the results on G1 and G2 Optane, graphs across different machines needs to be collected.
|  Figure info   | Contents  |
|  ----  | ----  |
| Raw log path  | `tmp/fastfair_original_test.log` and `tmp/fastfair_rap_mod_test.log` |
| Formatted data path  |  `output/case_study/fastfair_original_test.csv` and `output/case_study/fastfair_rap_mod_test.csv`|
| Graph path  |  `output/case_study/fastfair.png` |
| Plot script path  | `tools/plot_case_btree.py` |

The command to run a solitary test::

Original version:
```
numactl -N 0 build_cases/fastfair_original_test  -thread thread_num > name_of_the_log.log
```
Optimized version:
```
numactl -N 0 build_cases/fastfair_rap_mod_test  -thread thread_num > name_of_the_log.log
```

### Figure 13. Reducing read caused by prefetching.
This generated graph only shows the results on current machine. To compare the results on G1 and G2 Optane, graphs across different machines needs to be collected. Files are tagged with a timestamp in the format of "year-month-day-hour:minute:second".
|  Figure info   | Contents  |
|  ----  | ----  |
| Raw log path  | `tmp/prefetch_optimize.YYYY-MM-DD-HH:MM:SS.log` |
| Formatted data path  |  `output/case_study/prefetch_optimize.csv`|
| Graph path  |  `output/case_study/prefetch_optimize_read_size.png` |
| Plot script path  | function `general_task_runner()` in `run.py`  |

The command to run a solitary test::
 ```
 numactl -N 0 build_benchmark/bin/microbench  -test 8 > name_of_the_log.log
 ```
### Figure 14. Multithread read performance regarding avoiding prefetching.
This generated graph only shows the results on current machine. To compare the results on G1 and G2 Optane, graphs across different machines needs to be collected. Files are tagged with a timestamp in the format of "year-month-day-hour:minute:second".
|  Figure info   | Contents  |
|  ----  | ----  |
| Raw log path  | `tmp/read_throuput_against_prefetch.YYYY-MM-DD-HH:MM:SS.log`  |
| Formatted data path  |  `output/case_study/read_throuput_against_prefetch.csv`|
| Graph path  |  `output/case_study/prefetch_multithread_rd_lat.png` and `output/case_study/prefetch_multithread_rd_throughput.png` |
| Plot script path  | function `general_task_runner()` in `run.py`  |

The command to run a solitary test::
 ```
 numactl -N 0 build_benchmark/bin/microbench  -test 9 > name_of_the_log.log
 ```
## Notion
When the machine boots up, there could be unintended read or write operation on the DIMMs. If you observe strange spikes on the graph, you could wait for some time and run the code again.

The progress for case study is updated every 3 seconds. It's normal if some progress bars do not reach 100%.
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
