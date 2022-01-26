# directories used in cmake file
set(pm_util_dir ${PROJECT_SOURCE_DIR}/../pm_util)

add_definitions(-DRA_WORKCONFIG="${PROJECT_SOURCE_DIR}/cases/work.json")
add_definitions(-DPERSISTENT_POOL_DIR="/mnt/pmem/bench_map_file")
add_definitions(-DPERSISTENT_MAP_SIZE=\(1ULL<<34\))