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

add_subdirectory(${PROJECT_SOURCE_DIR}/../pm_util/ build)

include_directories(
    ${pm_util_dir}/include
)

set(CMAKE_CXX_FLAGS "-march=native")
add_definitions(-w)

# read buffer test
# set(rd_buf_src read_amp/main.cpp)
# add_executable(rd_buf ${rd_buf_src})
# target_link_libraries(rd_buf pthread vmem pm_util)

# add_definitions(-DPERSISTENT_MAP_SIZE=1000000000)


add_executable(microbench 
cases/main.cpp cases/read_after_flush.cpp 
cases/prefetching.cpp cases/read_buf_amp.cpp cases/lat.cpp
cases/seperate_rd_wr_buf.cpp cases/write_buffer.cpp)
target_link_libraries(microbench pm_util pthread gflags numa)
