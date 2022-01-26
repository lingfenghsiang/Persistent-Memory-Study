# directories used in cmake file
set(pm_util_dir ${PROJECT_SOURCE_DIR}/../pm_util)
# directories used in code
add_definitions(-DLOAD_FILE_DIR="${PROJECT_SOURCE_DIR}/../tmp/load.log")
add_definitions(-DRUN_FILE_DIR="${PROJECT_SOURCE_DIR}/../tmp/run.log")
add_definitions(-DPMEM_POOL_DIR="/mnt/pmem/")
add_definitions(-DPMEM_POOL_SIZE=0x400000000)