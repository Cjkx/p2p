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

#include "bm_types.h"
#include "mmio.h"


#define	CDMA_RX_CHANNEL		0x0
#define	CDMA_TX_CHANNEL		0x0


u32 sg_read32(uintptr_t  base, u32 offset);
void sg_write32(uintptr_t base, u32 offset, u32 value);
void* mmap_phy_addr(size_t length, off_t offset);
void munmap_dma_reg(size_t length, void* mapped_memory);






#endif