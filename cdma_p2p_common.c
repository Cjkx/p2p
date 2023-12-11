#include "cdma_p2p_common.h"



void sg_write32(uintptr_t base, u32 offset, u32 value)
{
	mmio_write_32((base + offset), value);
}

u32 sg_read32(uintptr_t  base, u32 offset)
{
	return mmio_read_32((base + offset));
}

void* mmap_phy_addr(size_t length, off_t offset) {
	int mem_fd;
    	void *mapped_memory;

	mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
	if (mem_fd == -1) {
		printf("Error opening /dev/mem");
		exit(EXIT_FAILURE);
   	}
	mapped_memory = mmap(NULL, length, PROT_READ | PROT_WRITE, 
			     MAP_SHARED, mem_fd, offset);
	if (mapped_memory == MAP_FAILED) {
		printf("Error mapping memory");
		close(mem_fd);
		exit(EXIT_FAILURE);
    	}
	close(mem_fd);

    	return mapped_memory;
}

void munmap_dma_reg(size_t length, void* mapped_memory) {
   	if (munmap(mapped_memory, length) == -1) {
		printf("Error unmapping memory");
		exit(EXIT_FAILURE);
    	}
}