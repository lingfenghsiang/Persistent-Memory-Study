# Persistent memory study

Implementaion of paper **Characterizing the Performance of Intel Optane Persistent Memory -- A Close Look at its On-DIMM Buffering**

This paper is to appear in EuroSys 2022.

This repository contains the code for microbenchmarks and case studies. It's recommended to execute the "run.py" script, because it
compiles the code and generate worloads for case studies. Please see [Usage](#usage).
## Table of Contents

<!-- - [Background](#background) -->
- [Prerequisites](#prerequisites)
- [Usage](#usage)
	- [Before Run](#before-run)
	- [Click and Run](#click-and-run)
  - [Details](#details)
- [Miscellaneous](#Miscellaneous)
<!-- ## Background -->

## Prerequisites

### Hardware requirements
Your machine needs to have Intel Optane DC Persistent Memory installed.
Besides, your machine also needs AVX512, CLWB, CLFLUSH and CLFLUSHOPT instruction sets.

It's encouraged to have at least 16GB space on the volatile file system `/dev/shm`, 
because the case study uses 16GB by default on to test CCEH performance. 
If you want to use less space, you could change that in file `micro_benchmarks/compiling_config.cmake`, or
you could also change that when you execute the program.
To check the size of your volatile file system you could use:
```
df -h | grep /dev/shm
```

Finally, it's also encouraged to install your persistent memory on NUMA node 0, if you have more than one socket, 
because the benchmark and case studies runs on node 0 by default.
To check where the persistent memory mounts, you may use
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

The persistent memory should be configured as non-interleaved for microbenchmarks and interleaved for case studies.

Set PM in non-interleaved mode:
```
# destroy current namespaces on persistent memory
ndctl destroy-namespace -f all
# reboot is required after this
ipmctl create -goal PersistentMemoryType=AppDirectNotInterleaved
# execute this after reboot
ndctl create-namespace
```
Set PM in interleaved mode:
```
# destroy current namespaces on persistent memory
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
1. Specify where the persistent memory pool locates for microbenchmarks.
2. Specify where the persistent memory pool locates for the case study.
3. Specify the location of your python with packages installed.
4. Set CPU in "performance" mode.
5. Initialize git submodules

If your PM device is mounted at "/mnt/pmem".
The default pool path for microbenchmarks is "/mnt/pmem/bench_map_file", specified in file 
`micro_benchmarks/compiling_config.cmake`, and that for the case study is "/mnt/pmem/", specified in
file `case_study/path.cmake`.

If your user name is `foo`, and your conda environment is installed at `/home/foo/anaconda3/bin/python`,
you need to specify this in file [run.py](run.py) at the beginning as
```
python_path = "/home/foo/anaconda3/bin/python"
```

```
# Set CPU in "performance" mode
echo "performance" | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
# Initialize git submodules
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
### Details
For more details about micro-benchmarks, please go to folder [micro_benchmarks](micro_benchmarks).

For more details about micro-benchmarks, please go to folder [case_study](case_study).

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
The offical ipmctl may not apply to 2nd generation Optane DC Persistent memory. If your ipmctl cannot configure the PM device, you need to download and compile the latest code of ipmctl. For more details please refer https://docs.pmem.io/ipmctl-user-guide/installing-ipmctl/building-and-installing-ipmctl-from-source-on-linux
