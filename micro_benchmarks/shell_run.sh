#!/usr/bin/sh

# throughput test
# for i in $(seq 6 6)
# do
# for j in $(seq 1 5)
# do
# build/bin/bw_bench -w 250M -t $j -i 20 -g $((2**$i)) -p seq
# done
# done

# read buffer test
# build/bin/rd_buf -a $((2**10)) $((2**15)) exp 2 -a $((13 * 2**10)) $((19 * 2**10)) linear 256 -a $((10 * 2**10)) $((13 * 2**10)) linear 1024 -a $((19 * 2**10)) $((32 * 2**10)) linear 4096

executable_dir=/home/xlf/documents/pmm_profiling/build/bin
# events=cpu-clock,mem_load_retired.local_pmm,LLC-load-misses,mem_load_retired.local_pmm,ocr.all_data_rd.pmm_hit_local_pmm.any_snoop

# perf_command="perf record -F 200 -e $events -g -o /dev/shm/perf.data --switch-output"
# numactl -N 0 $perf_command $executable_dir/microbench 2>err.log

${executable_dir}/bw_bench --wss=24K  --iter=500000

# for i in $(seq 12 30)
# do
# ${executable_dir}/bw_bench --wss=$((2**$i))B  --iter=$((2**(35-$i)))
# done
