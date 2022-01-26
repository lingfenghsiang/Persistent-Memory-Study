# Characterizing the Performance of Intel Optane Persistent Memory

# Requirements
## Hardware requirements
Please make sure your machine supports AVX512, CLWB, CLFLUSH and CLFLUSHOPT instruction sets. The persistent memory should be configured as non-interleaved, and we get all the results on a single Optane DC Persistent memory DIMM.

## Software requirements
Run on Ubuntu 20.04 LTS. Compatibility with other Linux distributions are not verified. Following packages needs to be installed.
```
sudo apt install libgflags-dev
```
Since the microbenchmark needs to call "ipmctl" to collect DIMM data, you need to run the code as "root" user.

# How to build
```
mkdir build
cd build && cmake ..
make
```
# How to run

# Available tests
## 0. read_buf_amp_tst
Prove the existence of PM read buffer and estimate its size.
## 1. trigger_prefetching,
Investigates the prefetching mechanism of PM device.

## 2. write_buffer,
Explore the write buffering mechanism.
## 3. write_buffer_flushing_period (not included in paper),
Estimate the flushing back period of fully written XP lines. To get the period, you should plot the write amplification with respect to the CPU cycles of inserted "meaningless" operations.
## 4. seperate_rd_wr_buf,
Verify that write and read buffers are seperated.
## 5. read_after_flush,
Estimate the latency of Read After Persist operations.

## 6. access_lat,
Estimate the latency of read, write operations under strict and relaxed persistency model.
# Collection test data


# Acknowledgements

I used some code in
[Json for mordern C++](https://github.com/nlohmann/json.git) by [Niels Lohmann](https://github.com/nlohmann) to parse my configurations.