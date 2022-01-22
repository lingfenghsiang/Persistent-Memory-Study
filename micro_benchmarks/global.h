#pragma once

#include <iostream>
#include <string>
#include <libvmem.h>
#include <string.h>
#include <locale>
#include <chrono>
#include <thread>
#include <vector>
#include <atomic>
#include <libpmem.h>

#define PERSISTENT_POOL_DIR "/mnt/pmem/xlf_map_file"
#define PERSISTENT_MAP_SIZE 1ULL<<34
#define VOLATILE_POOL_DIR "/mnt/pmem"
#define GET_PAGETABLE "/home/xlf/Documents/pmm_profiling/pagemap/pagemap "
#define POOL_SIZE 60000000000

