find_package (GTest REQUIRED)
find_package (Threads REQUIRED)

include_directories(
    ${GTEST_INCLUDE_DIRS}
)

add_subdirectory(test_libs)
add_subdirectory(test_vds_core)
add_subdirectory(test_vds_crypto)
add_subdirectory(test_vds_data)
add_subdirectory(test_vds_database)
add_subdirectory(test_vds_network)
add_subdirectory(test_vds_http)
add_subdirectory(test_vds_parser)
add_subdirectory(test_vds_scenarios)
