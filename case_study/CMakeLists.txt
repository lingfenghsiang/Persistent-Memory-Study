cmake_minimum_required(VERSION 3.0)
project(kv_test)

set(CXX_STANDARD 11)

if (NOT CMAKE_BUILD_TYPE)
    # set(CMAKE_BUILD_TYPE Release)
    # set(CMAKE_BUILD_TYPE Debug)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif ()

if (NOT USE_PMEM)
    # set(CMAKE_BUILD_TYPE Release)
    set(USE_PMEM ON)
    # set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif ()

include(path.cmake)

include_directories(
    ${pmdk_dir}/src/include
)
include_directories(
        ${xlf_util_dir}/include
)
link_directories(
    # "/home/xlf/Documents/pmdk-1.7.1/src/debug",
    ${pmdk_dir}/src/debug
)
link_directories(
    ${xlf_util_dir}/build
    ${PROJECT_SOURCE_DIR}/build/external_repo/cceh_mod
    ${PROJECT_SOURCE_DIR}/build/external_repo/cceh/CCEH-PMDK
)

set(CMAKE_CXX_FLAGS "-march=native")

set(src main.cpp)

# set(stl_hash_src src/stl_example/stl_hashtable.cpp)

# add_subdirectory(external_repo/cceh_mod)
add_subdirectory(external_repo/cceh/CCEH-PMDK)
add_subdirectory(external_repo/fastfair)

add_executable(${PROJECT_NAME} ${src})
# link_directories(
#     external_repo/dash/build
# )

# get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY LINK_DIRECTORIES)
# foreach(dir ${dirs})
#   message(STATUS "dir='${dir}'")
# endforeach()

target_link_libraries(${PROJECT_NAME} xlf_util tbb gflags pthread cceh_pmdk fastfair
 )
