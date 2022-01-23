cmake_minimum_required(VERSION 3.0)
project(cceh_pmdk)

set(CXX_STANDARD 11)

if (NOT CMAKE_BUILD_TYPE)
    # set(CMAKE_BUILD_TYPE Release)
    # set(CMAKE_BUILD_TYPE Debug)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif ()

include_directories(${PROJECT_SOURCE_DIR})

add_library(${PROJECT_NAME} SHARED src/CCEH.cpp wrap.cpp)
target_link_libraries(${PROJECT_NAME} pmem pmemobj rt)