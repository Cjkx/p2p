#ifndef __CDMA_P2P_COMMON_H__
#define __CDMA_P2P_COMMON_H__

#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <ctype.h>

#include "bm_types.h"
#include "mmio.h"


#define	CDMA_RX_CHANNEL		0x7
#define	CDMA_TX_CHANNEL		0x7




#define DEBUG

#ifdef DEBUG
#define debug_log(...)	printf(__VA_ARGS__)
#else
#define debug_log(...)	do {} while (0)
#endif

u32 sg_read32(uintptr_t  base, u32 offset);
void sg_write32(uintptr_t base, u32 offset, u32 value);
u32 sg_read8(uintptr_t  base, u32 offset);
void sg_write8(uintptr_t base, u32 offset, u32 value);
void* mmap_phy_addr(size_t length, off_t offset);
void munmap_dma_reg(size_t length, void* mapped_memory);
void p2p_sys_end_chain_config(uintptr_t cmd_reg_base);
int data_compare(uintptr_t src_ddr_addr_virtual, uintptr_t dst_ddr_value_virtual, uintptr_t dst_mem_start_addr, u32 data_length);
int data_compare_8(uintptr_t src_ddr_addr_virtual, uintptr_t dst_ddr_value_virtual, uintptr_t dst_mem_start_addr, u32 data_length);
void write_data_32(uintptr_t src_ddr_addr_virtual, uintptr_t dst_ddr_value_virtual, u32 data_length);
void write_data_8(uintptr_t src_ddr_addr_virtual, uintptr_t dst_ddr_value_virtual, u32 data_length);
int compare_frame(uintptr_t src_ddr_addr_virtual, uintptr_t dst_ddr_value_virtual, uintptr_t dst_mem_start_addr, u32 frame_length);
void write_frame(uintptr_t src_ddr_addr_virtual, uintptr_t dst_ddr_value_virtual, u32 frame_length);
void p2p_enable_desc(uintptr_t csr_reg_base, u8 mode);
int p2p_poll(uintptr_t csr_reg_base, u8 mode);
void dump(const void* ptr, size_t length);
int xlgmac_set_rx_crc_strip(char* xlgmac_mapped_memory);
int xlgmac_set_loopback_mode(char* xlgmac_mapped_memory, u8 mode);


#endif