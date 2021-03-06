project(vds_crypto CXX C)
cmake_minimum_required(VERSION 2.6.2)

if (UNIX)
	find_package (OpenSSL REQUIRED)
else()
	set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")
	find_package (OpenSSL REQUIRED)
endif (UNIX)


include_directories(${vds_core_SOURCE_DIR})
include_directories(${OPENSSL_INCLUDE_DIR})

FILE(GLOB HEADER_FILES *.h)
FILE(GLOB SOURCE_LIB *.cpp)

ADD_MSVC_PRECOMPILED_HEADER("stdafx.h" "stdafx.cpp" SOURCE_LIB)

add_library(vds_crypto STATIC ${SOURCE_LIB} ${HEADER_FILES})
target_link_libraries(
  vds_crypto
  vds_core
  ${OPENSSL_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
)
