project(vds_node CXX C)
cmake_minimum_required(VERSION 2.6.2)

include_directories(${vds_core_SOURCE_DIR})
include_directories(${vds_http_SOURCE_DIR})
include_directories(${vds_network_SOURCE_DIR})

FILE(GLOB HEADER_FILES *.h)
FILE(GLOB SOURCE_FILES *.cpp)
ADD_MSVC_PRECOMPILED_HEADER("stdafx.h" "stdafx.cpp" SOURCE_FILES)
add_executable(vds_node ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(vds_node vds_core vds_network vds_http ${CMAKE_THREAD_LIBS_INIT})

