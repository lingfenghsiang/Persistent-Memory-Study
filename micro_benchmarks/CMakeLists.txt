cmake_minimum_required(VERSION 3.0)
project(profiling_pmm)

include(compiling_config.cmake)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

if( NOT CMAKE_BUILD_TYPE )
    message(STATUS "No build type selected, default to Release")
    # set( CMAKE_BUILD_TYPE Release)
    set( CMAKE_BUILD_TYPE Debug)
    # set( CMAKE_BUILD_TYPE RelWithDebInfo)
else()
    message(STATUS "Build type is set to ${CMAKE_BUILD_TYPE}")
endif()

include_directories(
    ${xlf_util_dir}/include
)
link_directories(
    ${xlf_util_dir}/build
)

set(CMAKE_CXX_FLAGS "-march=native")
add_definitions(-w)


add_custom_command(OUTPUT cpu_info.config
        COMMAND cd ${PROJECT_SOURCE_DIR} && ./generate_cpu_info.sh
        VERBATIM
        )
# throughput test
set(rd_src throughput/main.cpp throughput/experiment_sets.cpp)
add_executable(bw_bench ${rd_src} cpu_info.config)
target_link_libraries(bw_bench pthread vmem xlf_util gflags)
# read buffer test
set(rd_buf_src read_amp/main.cpp)
add_executable(rd_buf ${rd_buf_src})
target_link_libraries(rd_buf pthread vmem xlf_util)


# extreme cases
# add_executable(rd_buf_case0 cases/main.cpp cases/read_buf_case_0.cpp cases/prefetching.cpp)
# target_link_libraries(rd_buf_case0 pthread vmem xlf_util)

# add_executable(prefetching cases/main.cpp cases/prefetching.cpp)
# target_link_libraries(prefetching pthread vmem xlf_util)

add_executable(microbench 
cases/main.cpp cases/read_after_flush.cpp cases/prefetching.cpp cases/read_buf_amp.cpp cases/rd_wr_lat.cpp 
cases/read_and_write_bw.cpp cases/read_buf_case_0.cpp cases/lat.cpp
cases/seperate_rd_wr_buf.cpp cases/write_buffer.cpp)
target_link_libraries(microbench vmem xlf_util pthread gflags numa)

# add_executable(write_buffer cases/main.cpp cases/write_buffer.cpp)
# target_link_libraries(write_buffer vmem xlf_util pthread pmem)

# add_executable(seperate_rd_wr_buf cases/main.cpp cases/seperate_rd_wr_buf.cpp)
# target_link_libraries(seperate_rd_wr_buf vmem xlf_util pthread)

# add_executable(read_and_write_bw cases/main.cpp cases/read_and_write_bw.cpp)
# target_link_libraries(read_and_write_bw vmem xlf_util pthread)