cmake_minimum_required(VERSION 3.5)

set (CMAKE_C_COMPILER 
     "/opt/gcc-riscv64-unknown-linux-gnu/bin/riscv64-unknown-linux-gnu-gcc")

#set (CMAKE_C_COMPILER "/usr/bin/gcc")

project (test_cdma_p2p)

include_directories(include)

set(SOURCES  cdma_p2p_common.c cdma_pmu.c )

add_library(libp2p STATIC  raw_tcp_ip.c)



add_executable(test_xlgmac_core ${SOURCES} xlgmac_core.c)
add_executable(test_p2p_normal ${SOURCES} test_p2p_normal.c)
add_executable(test_p2p_tcp   ${SOURCES} test_p2p_tcp.c)

target_link_libraries(test_p2p_normal libp2p)
target_link_libraries(test_p2p_tcp libp2p)

# target_include_directories(test_cdma_p2p PRIVATE ${PROJECT_SOURCE_DIR})