# directories used in cmake file
# set(pmdk_dir /home/xlf/documents/pmdk)
set(xlf_util_dir /home/xlf/Documents/MyUtils)
# directories used in code
add_definitions(-DLOAD_FILE_DIR="/home/xlf/Documents/prepare_ycsb/load.log")
add_definitions(-DRUN_FILE_DIR="/home/xlf/Documents/prepare_ycsb/run.log")
add_definitions(-DPMEM_POOL_DIR="/mnt/pmem/")
add_definitions(-DPMEM_POOL_SIZE=0x400000000)