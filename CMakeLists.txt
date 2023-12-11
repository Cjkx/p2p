cmake_minimum_required(VERSION 3.5)

set (CMAKE_C_COMPILER 
     "/opt/gcc-riscv64-unknown-linux-gnu/bin/riscv64-unknown-linux-gnu-gcc")

project (test_cdma_p2p)


set(SOURCES test_cdma_p2p.c cdma_p2p_common.c xlgmac_vt.c)

add_executable(test_cdma_p2p ${SOURCES})

# target_include_directories(test_cdma_p2p PRIVATE ${PROJECT_SOURCE_DIR})