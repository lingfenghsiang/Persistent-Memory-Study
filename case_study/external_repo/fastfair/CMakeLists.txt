cmake_minimum_required(VERSION 3.0)
project(cceh)

# include(path.cmake)
include_directories(
        ${PROJECT_SOURCE_DIR}
        ${xlf_util_dir}/include
        "./"
)

link_directories(
        ${xlf_util_dir}/build
)


set(CMAKE_CXX_STANDARD 11)
if (NOT CMAKE_BUILD_TYPE)
#     set(CMAKE_BUILD_TYPE Debug)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif ()

execute_process(COMMAND cat /proc/cpuinfo COMMAND grep clflush OUTPUT_VARIABLE ENABLE_CLFLUSH)
execute_process(COMMAND cat /proc/cpuinfo COMMAND grep clflushopt OUTPUT_VARIABLE ENABLE_CLFLUSHOPT)
execute_process(COMMAND cat /proc/cpuinfo COMMAND grep clwb OUTPUT_VARIABLE ENABLE_CLWB)
execute_process(COMMAND cat /proc/cpuinfo COMMAND grep avx2 OUTPUT_VARIABLE ENABLE_AVX2)
execute_process(COMMAND cat /proc/cpuinfo COMMAND grep bmi2 OUTPUT_VARIABLE ENABLE_BMI2)
execute_process(COMMAND cat /proc/cpuinfo COMMAND grep avx512 OUTPUT_VARIABLE ENABLE_AVX512)

if(ENABLE_CLWB)
    add_definitions(-DCLWB)
    message(STATUS "Looking for clwb instruction - found")
elseif(ENABLE_CLFLUSHOPT)
    add_definitions(-DCLFLUSH_OPT)
    message(STATUS "Looking for clwb instruction - not found")
    message(STATUS "Looking for clflushopt instruction - found")
elseif(ENABLE_CLFLUSH)
    add_definitions(-DCLFLUSH)
    message(STATUS "Looking for clwb instruction - not found")
    message(STATUS "Looking for clflushopt instruction - not found")
    message(STATUS "Looking for clflush instruction - found")
else()
    message(FATAL_ERROR "Cannot find any flush instructions (clflush, clflushopt, clwb)")
endif()

if(ENABLE_AVX2)
    message(STATUS "Looking for avx2 instruction - found")
else()
    message(STATUS "Looking for avx2 instruction - not found")
endif()

if(ENABLE_BMI2)
    message(STATUS "Looking for bmi2 instruction - found")
else()
    message(STATUS "Looking for bmi2 instruction - not found")
endif()

if(ENABLE_AVX512)
    set(CMAKE_CXX_FLAGS "-mavx512f -mavx512vl -mavx512bw -mavx512dq -mavx512cd ${CMAKE_CXX_FLAGS}")
    add_definitions(-DUSE_AVX512)
    message(STATUS "Looking for avx512 instruction - found")
else()
    message(STATUS "Looking for avx512 instruction - not found")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")


add_library(fastfair_original SHARED wrap.cpp)
target_link_libraries(fastfair_original pthread vmem crypto)

add_library(fastfair_rap_mod SHARED wrap.cpp)
target_link_libraries(fastfair_rap_mod pthread vmem crypto)
target_compile_definitions(fastfair_rap_mod
PUBLIC -DRAP_MOD)
