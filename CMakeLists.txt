cmake_minimum_required(VERSION 3.5)

set (CMAKE_C_COMPILER 
     "/opt/gcc-riscv64-unknown-linux-gnu/bin/riscv64-unknown-linux-gnu-gcc")

#set (CMAKE_C_COMPILER "/usr/bin/gcc")

project (test_cdma_p2p)


set(SOURCES  cdma_p2p_common.c xlgmac_core.c)

add_executable(test_p2p_normal ${SOURCES} test_cdma_p2p.c)
add_executable(test_p2p_tcp ${SOURCES} test_p2p_tcp.c)

# target_include_directories(test_cdma_p2p PRIVATE ${PROJECT_SOURCE_DIR})