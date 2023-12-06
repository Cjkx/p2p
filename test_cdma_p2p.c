#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdio.h>


#include "mmio.h"
#include "cdma_p2p_normal.h"
#include "bm_types.h"

void sg_write32(uintptr_t base, u32 offset, u32 value)
{
	mmio_write_32((base + offset), value);
}

u32 sg_read32(uintptr_t  base, u32 offset)
{
	return mmio_read_32((base + offset));
}

void* mmap_dma_phy_addr(void) {
	int mem_fd;
    	void *mapped_memory;
	mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
	if (mem_fd == -1) {
		printf("Error opening /dev/mem");
		exit(EXIT_FAILURE);
   	}
	mapped_memory = mmap(NULL, CDMA_MEMORY_RANGE, PROT_READ | PROT_WRITE, MAP_SHARED, 
				mem_fd, CDMA_PHY_ADDRESS_START);
	if (mapped_memory == MAP_FAILED) {
		printf("Error mapping memory");
		close(mem_fd);
		exit(EXIT_FAILURE);
    	}
	close(mem_fd);

    	return mapped_memory;
}

void* mmap_ddr_phy_addr(void) {
	int mem_fd;
    	void *mapped_memory;
	mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
	if (mem_fd == -1) {
		printf("Error opening /dev/mem");
		exit(EXIT_FAILURE);
   	}
	mapped_memory = mmap(NULL, DDR_MEMORY_RANGE, PROT_READ | PROT_WRITE, MAP_SHARED, 
				mem_fd, DDR_PHY_ADDRESS_START);
	if (mapped_memory == MAP_FAILED) {
		printf("Error mapping memory");
		close(mem_fd);
		exit(EXIT_FAILURE);
    	}
	close(mem_fd);

    	return mapped_memory;
}


void munmap_dma_reg(void* mapped_memory, size_t size) {
   	if (munmap(mapped_memory, size) == -1) {
		printf("Error unmapping memory");
		exit(EXIT_FAILURE);
    	}
}

static void cdma_p2p_normal_csr_config(uintptr_t csr_reg_base){
	u32 reg_val = 0;

	/* enable replay function */
	/* enable hardware replay function*/
	reg_val = sg_read32(csr_reg_base, CDMA_CSR_0_OFFSET);
	reg_val |= ((1 << REG_CONNECTION_ON) | (1 << REG_HW_REPLAY_EN));
	sg_write32(csr_reg_base, CDMA_CSR_0_OFFSET,reg_val);
	/* when time cnt is bigger than this reg, trigger timeout replay*/
	//reg_val = sg_read32(csr_reg_base, CDMA_CSR_5_OFFSET);
	reg_val = 0;
	reg_val |= 0xfffff;
	sg_write32(csr_reg_base, CDMA_CSR_5_OFFSET,reg_val);
	/* same subinst replay max times*/
	reg_val = sg_read32(csr_reg_base, CDMA_CSR_4_OFFSET);
	reg_val &= ~(0xf << REG_REPLAY_MAX_TIME);
	sg_write32(csr_reg_base, CDMA_CSR_4_OFFSET,reg_val);
	/* config  VLAN_ID */

	/* enable VLAN_ID */

	/* CRC Pad Control*/

	/* SA Insertion Control*/

	/* Checksum Insertion Control or TCP Payload Length*/

	/* sec_length*/
	reg_val = sg_read32(csr_reg_base, CDMA_CSR_4_OFFSET);
	reg_val &= ~(7 << REG_SEC_LENGTH);
	reg_val |= (5 << REG_SEC_LENGTH);
	sg_write32(csr_reg_base, CDMA_CSR_4_OFFSET,reg_val);
	/*when occur replay overflow, set this reg to recontinue*/

	/*CDMA own tx channel id for mtl fab*/
	/*CDMA own rx channel id for mtl fab*/
	/*CDMA EDMA MTL FAB Arbitration：
	00：fix priority，CDMA high priority
	01：fix priority，EDMA high priority
	10：round robin
	*/
	reg_val = sg_read32(csr_reg_base, MTL_FAB_CSR_00);
	reg_val &= ~( 0xf << REG_CDMA_TX_CH_ID);
	reg_val |=  ( 0xf << REG_CDMA_TX_CH_ID);
	reg_val &= ~( 0xf << REG_CDMA_RX_CH_ID);
	reg_val |=  ( 0xf << REG_CDMA_RX_CH_ID);
	reg_val &= ~( 0x1 << REG_CDMA_FAB_BYPASS);
	reg_val &= ~( 0x3 << REG_CDMA_FAB_ARBITRATION);
	reg_val |=  ( 0x1 << REG_CDMA_FAB_ARBITRATION);
	sg_write32(csr_reg_base, MTL_FAB_CSR_00, reg_val);
	/*配置数据读到对应的网络,建议配置c2c_rn
	 *低4bit为芯片级联信息，建议配置为0000
	 *CXP: 	c2c_rn, addr[15:12]=1000
   	 *	c2c_rni, addr[15:12]=1001
	 */
	reg_val = sg_read32(csr_reg_base, CDMA_CSR_141_OFFSET);
	reg_val &= ~(0xff << REG_INTRA_DIE_WRITE_ADDR_H8);
	reg_val |= (0x80 << REG_INTRA_DIE_WRITE_ADDR_H8);
	sg_write32(csr_reg_base, CDMA_CSR_141_OFFSET, reg_val);
	/*配置数据从对应网络读出,根据需求配置，建议配置c2c_rn
	 *低4bit为芯片级联信息，建议配置为0000
	 *CXP: 	c2c_rn, addr[15:12]=1000
   	 *	c2c_rni, addr[15:12]=1001
	 */
	reg_val = sg_read32(csr_reg_base, CDMA_CSR_141_OFFSET);
	reg_val &= ~(0xff << REG_INTRA_DIE_READ_ADDR_H8);
	reg_val |= (0x80 << REG_INTRA_DIE_READ_ADDR_H8);
	sg_write32(csr_reg_base, CDMA_CSR_141_OFFSET, reg_val);
}


/*
	（1）	src数据格式可为8bit/16bit32bit
	（2）	src支持GMEM/L2M
	（3）	dst支持Ethernet（对端GMEM/L2M/）
*/
int testcase_cdma_p2p_normal_pio(char* dma_base_mmap,
				 u64 src_mem_start_addr,
				 int src_format,
				 u32 data_length,
				 u64 dst_mem_start_addr)
{
	u32 reg_val = 0;
	u32 mem_tag = 0;
	u64 low = 0, high = 0;
	u64 src_addr_h13 = ((src_mem_start_addr >> 32) | (mem_tag << 8)) & 0x1FFF;
	u64 src_addr_l32 = (src_mem_start_addr ) & 0xffffffff;
	u64 dst_addr_h13 = (dst_mem_start_addr >> 32) & 0x1FFF;
	u64 dst_addr_l32 = (dst_mem_start_addr ) & 0xffffffff;
	u32 count = 1000;

	printf("src_mem_start_addr:0x%lx,src_addr_h13:0x%lx src_addr_l32 :0x%lx,\n\
		dst_mem_start_addr:0x%lx,dst_addr_h13:0x%lx,dst_addr_l32:0x%lx\n",
		src_mem_start_addr,src_addr_h13,src_addr_l32,\
		dst_mem_start_addr,dst_addr_h13,dst_addr_l32);
	
	uintptr_t cmd_reg_base = (uintptr_t)(dma_base_mmap + CMD_REG_OFFSET);
	uintptr_t desc_updt = (uintptr_t)(dma_base_mmap + DESC_UPDT_OFFSET);
	uintptr_t csr_reg_base = (uintptr_t)(dma_base_mmap + CSR_REG_OFFSET);
	printf("cmd_reg_base:0x%lx, desc_updt:0x%lx, csr_reg_base :0x%lx\n ", \
		cmd_reg_base, desc_updt, csr_reg_base);

	cdma_p2p_normal_csr_config(csr_reg_base);

	low = (1ull << CDMA_CMD_INTR_ENABLE) |
	      ((u64)CDMA_GENERAL << CDMA_CMD_CMD_TYPE) |
	      ((u64)src_format << CDMA_CMD_SRC_DATA_FORMAT) |
	      (src_addr_h13 << CDMA_CMD_SRC_START_ADDR_H13) |
	      (dst_addr_h13 << CDMA_CMD_DST_START_ADDR_H13);
	mmio_write_64(cmd_reg_base,low);
	u64 low_val = mmio_read_64(cmd_reg_base);
	printf("function:%s, line:%d, low :0x%lx\n", __func__, __LINE__, low);
	printf("function:%s, line:%d, low_val = 0x%lx\n", __func__, __LINE__, low_val);

	high = (data_length << CDMA_CMD_CMD_LENGTH) |
	       (src_addr_l32 << CDMA_CMD_SRC_START_ADDR_L32) ;
	mmio_write_64(cmd_reg_base + 0x8,high);
	u64 high_val = mmio_read_64(cmd_reg_base + 0x8);
	printf("function:%s, line:%d, hign :0x%lx\n",__func__, __LINE__, low);
	printf("function:%s, line:%d, hign_val = 0x%lx\n",__func__, __LINE__, high_val);

	low = (dst_addr_l32 << CDMA_CMD_DST_START_ADDR_L32);
	mmio_write_64(cmd_reg_base + 0x10, low);
	low_val = mmio_read_64(cmd_reg_base + 0x10);
	printf("function:%s, line:%d, low :0x%lx\n", __func__, __LINE__, low);
	printf("function:%s, line:%d, low_val = 0x%lx\n", __func__, __LINE__, low_val);

	/* In pio mode write 1 to update des*/
	sg_write32(desc_updt, REG_DESCRIPTOR_UPDATE, 1);
	/* poll */
	while (sg_read32(csr_reg_base,CDMA_CSR_20_Setting) & 0x1 != 0x1){
		usleep(1);
		if (--count == 0) {
			printf("cdma polling wait timeout\n");
			return -1;
		}
	}
	 return 0;
}
#if 0
static void cdma_p2p_desc_csr_config(u32 desc_addr_l32,u32 desc_addr_h1){
	u32 reg_val = 0;

	/*des axi max arlen*/
	reg_val = sg_read32(CSR_REG_BASE, CDMA_CSR_4_OFFSET);
	reg_val |= (7 << REG_DES_MAX_ARLEN);
	sg_write32(CSR_REG_BASE, CDMA_CSR_4_OFFSET, reg_val);
	/* L32 addr [38:7]*/
	sg_write32(CSR_REG_BASE, CDMA_CSR_9_OFFSET, desc_addr_l32);
	/* h1 addr [39:39]*/
	sg_write32(CSR_REG_BASE, CDMA_CSR_9_OFFSET, desc_addr_h1);
	/* reg_des_arcache  reg_des_arqos default*/

	/* desc net config*/
	reg_val = sg_read32(CSR_REG_BASE, CDMA_CSR_142_OFFSET);
	reg_val &= ~(0xff << reg_des_read_addr_h8);
	reg_val |= (0x80 << reg_des_read_addr_h8);
	sg_write32(CSR_REG_BASE, CDMA_CSR_142_OFFSET, reg_val);
}
int testcase_cdma_p2p_normal_desc(int desc_num,
				 u64 src_mem_start_addr,
				 int src_format,
				 u32 data_length
				 u64 dst_mem_start_addr)
{
	cdma_p2p_normal_csr_config();
	uintptr_t  physicalAddress = 0x100000000;
	dma_general_desc* dma_general_desc_addr = (dma_general_desc*)physicalAddress;//4GB
	u32 dma_general_desc_addr_l32 = (physicalAddress >> 7) & 0xFFFFFFFF;
	u32 dma_general_desc_addr_h1 = (physicalAddress >> 39) & 0x1;
	cdma_p2p_desc_csr_config(dma_general_desc_addr_l32,dma_general_desc_addr_h1);
	/*At last,need add cdma_sys cmd */
	for(int i = 0;i < desc_num ;i++){
		if(i == desc_num - 1)
			dma_general_desc_addr->intr_en = 1;
		else
			dma_general_desc_addr->intr_en = 0;
		dma_general_desc_addr->cmd_type = (u32)CDMA_GENERAL ;
		dma_general_desc_addr->src_data_format = src_format;
		dma_general_desc_addr->src_start_addr_l32 = ;

	}

}

#endif

int main()
{
	int src_format;
	u32 data_length = 1;
	u64 dst_mem_start_addr = 0x0;


	char* dma_mapped_memory = mmap_dma_phy_addr();
	char* ddr_mapped_memory = mmap_ddr_phy_addr();
	
	uintptr_t ddr_value_virtual = (uintptr_t)(ddr_mapped_memory + 0x1000);
 	uintptr_t ddr_value_phy = DDR_PHY_ADDRESS_START + DDR_PHY_OFFSET;
	printf("ddr_value_virtual:0x%lx\n", ddr_value_virtual);

	sg_write32(ddr_value_virtual,0x00,0x13);
	u32 ddr_value = sg_read32(ddr_value_virtual,0x00);
	printf("ddr_value:%u\n", ddr_value);
	
  	testcase_cdma_p2p_normal_pio((char*)dma_mapped_memory,
				(u64)ddr_value_phy,
				(int)DATA_INT32,
				data_length,
				dst_mem_start_addr);
	return 0;
}
