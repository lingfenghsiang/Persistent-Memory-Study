# Microbenchmark

# How to build
```
mkdir build
cd build && cmake ..
make
```
# How to run
To run the code, just use

`build/bin/microbench -[options] [value]`

The options include:
- pool_dir: The location of mapped file.
- max_size: The size of max addressable space on persistent memory
- pmm: Whether to run the bench mark on persistent memory. If yes type "-pmm", otherwise type "-nopmm"
- test: Which test to run. Available tests are shown below.
# Available tests
## 0. read_buf_amp_tst
Prove the existence of PM read buffer and estimate its size.
## 1. trigger_prefetching,
Investigates the prefetching mechanism of PM device. If you run this test, you must turn off the CPU prefetching in BIOS configuration.

## 2. write_buffer,
Explore the write buffering mechanism and its size. If you want its working set size set flexibly, you may configure its working set size in file [work.json](cases/work.json). Here it is enclosed. "inc_type" hints how you like the working set size increments. It could be either exponential("exp") or linear("linear"). You should also specify the beginning, the end and the step it takes as the working set size gradually increases.

```
{
    "jobs": [
        {
            "start_wss": 4096,
            "end_wss": 12288,
            "inc_type": "exp",
            "stride": 2
        },
        {
            "start_wss": 12288,
            "end_wss": 18432,
            "inc_type": "linear",
            "stride": 256
        },
        {
            "start_wss": 18432,
            "end_wss": 32768,
            "inc_type": "linear",
            "stride": 1024
        }
    ]
}

```
## 3. write_buffer_flushing_period (not included in paper),
Estimate the flushing back period of fully written XP lines. To get the period, you should plot the write amplification with respect to the CPU cycles of inserted "meaningless" operations.
When the execution is over, you get write amplification period and count_loop. You should plot the relation between write amplification and period.
The count_loop determins the period, while period is how much CPU cycles have been inserted between two write operations.
## 4. seperate_rd_wr_buf,
Verify that write and read buffers are seperated.
You should focus on the log, the working set size for write is smaller than write buffer and the working set size for read is also smaller than read buffer.
However, the sum of the two working set size is larger than any buffer. If the mixed read size and write size are equal to the sum of the sole read and write, it means read and write buffers are seperate.
## 5. read_after_flush,
Estimate the latency of Read After Persist operations.

## 6. access_lat,
Estimate the latency of read, write operations under strict and relaxed persistency model.

# Acknowledgements

I used some code in
[Json for mordern C++](https://github.com/nlohmann/json.git) by [Niels Lohmann](https://github.com/nlohmann) to parse my configurations in write buffer tests.