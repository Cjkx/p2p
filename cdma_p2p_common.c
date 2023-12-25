#include "cdma_p2p_common.h"
#include "cdma_p2p_normal.h"
#include <ctype.h>

void sg_write32(uintptr_t base, u32 offset, u32 value)
{
	mmio_write_32((base + offset), value);
}

u32 sg_read32(uintptr_t  base, u32 offset)
{
	return mmio_read_32((base + offset));
}

void sg_write8(uintptr_t base, u32 offset, u32 value)
{
	mmio_write_8((base + offset), value);
}

u32 sg_read8(uintptr_t  base, u32 offset)
{
	return mmio_read_8((base + offset));
}

void dump(const void* ptr, size_t length) {
    const unsigned char* bytes = (const unsigned char*)ptr;
    size_t col,row;
    printf("   | ");
    for (int i = 0; i < 16; i++) {
        printf("%02d ", i);
    }
    printf("\n");
    printf("---|-------------------------------------\n");

    for (row = 0; row < length; row += 16) {
        printf("%02d | ", (int)row);
        for (col = 0; col < 16; col++) {
            if (row + col < length) {
                printf("%02x ", bytes[row + col]);
            } else {
                printf("   ");
            }
        }
	for (col = 0; col < 16; col++)
            if (row + col < length)
                printf("%c", isprint(bytes[row+col]) ? bytes[row+col] : '.');
        printf("\n");
	
    }
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

int data_compare(uintptr_t src_ddr_addr_virtual,
		 uintptr_t dst_ddr_value_virtual,
		 uintptr_t dst_mem_start_addr,
		 u32 data_length){
	u32 src_val;
	u32 dst_val;

	for (int i = 0; i < data_length; i++) {
		src_val = sg_read32(src_ddr_addr_virtual, i * 4);
		dst_val = sg_read32(dst_ddr_value_virtual, i * 4);
		if (src_val != dst_val) {
			printf("data_compare error at addr 0x%lx exp 0x%x got 0x%x\n",
				dst_mem_start_addr + 4 * i, src_val, dst_val);
			return -1;
		}
		if (i < 128) {
			printf("0x%x ",dst_val);
			if (i % 16 == 15)
				printf("\n");
		}
	}
	printf("data_compare_32 Successful!\n");
	return 0;
}

int data_compare_8(uintptr_t src_ddr_addr_virtual,
		 uintptr_t dst_ddr_value_virtual,
		 uintptr_t dst_mem_start_addr,
		 u32 data_length){
	u32 src_val;
	u32 dst_val;

	for (int i = 0; i < data_length; i++) {
		src_val = sg_read8(src_ddr_addr_virtual, i);
		dst_val = sg_read8(dst_ddr_value_virtual, i);
		if (src_val != dst_val) {
			printf("data_compare error at addr 0x%lx exp 0x%x got 0x%x\n",
				dst_mem_start_addr + i, src_val, dst_val);
			return -1;
		}
	}
	printf("data_compare_8 Successful!\n");
	return 0;
}



void write_data_32(uintptr_t src_ddr_addr_virtual, 
		       uintptr_t dst_ddr_value_virtual, 
		       u32 data_length) {

	for(int i = 0; i < data_length ; i++) {
		sg_write32(src_ddr_addr_virtual, i * 4, i);
		sg_write32(dst_ddr_value_virtual, i * 4 ,0);
	}
}

void write_data_8(uintptr_t src_ddr_addr_virtual, 
		       uintptr_t dst_ddr_value_virtual, 
		       u32 data_length) {

	for(int i = 0; i < data_length ; i++) {
		sg_write8(src_ddr_addr_virtual, i , i);
		sg_write8(dst_ddr_value_virtual, i , 0);
	}
}


void write_frame(uintptr_t src_ddr_addr_virtual, 
			uintptr_t dst_ddr_value_virtual, 
			u32 frame_length) {
	u32 width32_num = frame_length / 4;
	u32 byte_num = frame_length % 4 ;
	
	write_data_32(src_ddr_addr_virtual , 
		      dst_ddr_value_virtual , 
		      width32_num);
	write_data_8(src_ddr_addr_virtual + 4 * width32_num,
		     dst_ddr_value_virtual + 4 * width32_num, 
		     byte_num);
	dump((void*)src_ddr_addr_virtual, 128);
}

int compare_frame(uintptr_t src_ddr_addr_virtual, 
			 uintptr_t dst_ddr_value_virtual, 
			 uintptr_t dst_mem_start_addr, 
			 u32 frame_length) {
	int ret;

	u32 width32_num = frame_length / 4;
	u32 byte_num = frame_length % 4;

	ret = data_compare(src_ddr_addr_virtual, 
			   dst_ddr_value_virtual, 
	 		   dst_mem_start_addr, 
			   width32_num);
	if(ret < 0)
		return ret;
	
	ret = data_compare_8(src_ddr_addr_virtual + 4 * width32_num, 
			     dst_ddr_value_virtual +  4 * width32_num, 
	 		     dst_mem_start_addr + 4 * width32_num, 
			     byte_num);
	if(ret == 0)
		printf("cdma transfer successful!\n");
	return ret;
}

void p2p_enable_desc(uintptr_t csr_reg_base, u8 mode) {
	u32 reg_val;
	
	if (TCP_SEND == mode) {
		reg_val = sg_read32(csr_reg_base, TCP_CSR_02_Setting);
		reg_val |= (1 << REG_TCP_SEND_DES_MODE_ENABLE);
		sg_write32(csr_reg_base, TCP_CSR_02_Setting, reg_val);
		printf("enable tcp_send desc!\n");
	} else if (TCP_RECEIVE == mode) {
		reg_val = sg_read32(csr_reg_base, TCP_CSR_02_Setting);
		reg_val |= (1 << REG_TCP_RECEIVE_DES_MODE_ENABLE);
		sg_write32(csr_reg_base, TCP_CSR_02_Setting, reg_val);
		printf("enable tcp_rcv desc!\n");
	} else if (NORMAL_GENERAL_DESC == mode) {
		reg_val = sg_read32(csr_reg_base, CDMA_CSR_0_OFFSET);
		reg_val |= (1 << REG_DES_MODE_ENABLE);
		sg_write32(csr_reg_base, CDMA_CSR_0_OFFSET, reg_val);
		printf("enable normal desc\n");
	}
	
}

int p2p_poll(uintptr_t csr_reg_base, u8 mode) {
	u32 reg_offset;
	u32 bit_offset;
	u32 count = 1500;
	char* mode_name;
	u8 poll_val;

	switch (mode)
	{
		case NORMAL_GENERAL_PIO:
			reg_offset = CDMA_CSR_20_Setting;
			bit_offset = INTR_CMD_DONE_STATUS;
			mode_name = "NORMAL_GENERAL_PIO";
			poll_val = 1;
			break;
		case NORMAL_GENERAL_DESC:
			reg_offset = CDMA_CSR_0_OFFSET;
			bit_offset = REG_DES_MODE_ENABLE;
			mode_name = "NORMAL_GENERAL_DESC";
			poll_val = 0;
			break;
		case TCP_SEND:
			reg_offset = CDMA_CSR_20_Setting;
			bit_offset = INTR_TCP_SEND_CMD_DONE;
			mode_name = "TCP_SEND";	
			poll_val = 1;
			break;
		case TCP_RECEIVE:
			reg_offset = CDMA_CSR_20_Setting;
			bit_offset = INTR_TCP_RCV_CMD_DONE;
			mode_name = "TCP_RECEIVE";
			poll_val = 1;
			break;	
		default:
			break;
	}

	while(((sg_read32(csr_reg_base, reg_offset) >> bit_offset) & 0x1) != poll_val){
		usleep(1);
		if (--count == 0) {
			printf("[%s]cdma polling wait timeout\n",mode_name);
			return -1;
		}
		if (count % 100 == 0){
			printf("[%s]cdma transfer consumes %d us\n", 
				mode_name, 1500 - count);
		}
	}

	printf("[%s] CDMA polling complete\n",mode_name);

	return 0;
}


void p2p_sys_end_chain_config(uintptr_t cmd_reg_base) {
	u64 reg_val;

	reg_val = (1ull << CDMA_CMD_INTR_ENABLE) |
	          ((u64)CDMA_SYS << CDMA_CMD_CMD_TYPE) |
		  ((u64)CHAIN_END << CDMA_CMD_SPECIAL_FUNCTION);

	mmio_write_64(cmd_reg_base, reg_val);
	printf("[CDMA_CMD_0]:0x%lx\n", reg_val);
}

