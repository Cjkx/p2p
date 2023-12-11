#include "cdma_p2p_common.h"
#include "cdma_p2p_normal.h"


static void eth_header_construction(uintptr_t csr_reg_base) {
	int val;
	u8  vlan_header[4];
	u32 read_val;
	eth_header_t eth_header = {
		.dest_mac = {0x00, 0x1b, 0x21, 0x3b, 0x30, 0x8e},
		.source_mac = {0xe0, 0xa5, 0x09, 0x00, 0x2a, 0x15},
		.vlan_header = {
			.priority = 0,
			.vlan_id = 0,
			.cfi = 0,
			.ether_type = 0x8100
		}
	};
	val = (eth_header.dest_mac[3] << 24) | (eth_header.dest_mac[2] << 16) |
	      (eth_header.dest_mac[1] << 8) | eth_header.dest_mac[0] ;
	sg_write32(csr_reg_base, CDMA_CSR_148_Setting, val);
	printf("[CDMA_CSR_148_Setting] expected value:0x%x\n", val);
	read_val = sg_read32(csr_reg_base, CDMA_CSR_151_Setting);
	printf("[CDMA_CSR_148_Setting] real value:0x%x\n", read_val);
	
	val = (eth_header.source_mac[1] << 24) | (eth_header.source_mac[0] << 16) |
		(eth_header.dest_mac[4] << 8) | eth_header.dest_mac[5];
	sg_write32(csr_reg_base, CDMA_CSR_149_Setting, val);
	printf("[CDMA_CSR_149_Setting] expected value:0x%x\n", val);
	read_val = sg_read32(csr_reg_base, CDMA_CSR_151_Setting);
	printf("[CDMA_CSR_149_Setting] real value:0x%x\n", read_val);

	val = (eth_header.source_mac[5] << 24) | (eth_header.source_mac[4] << 16) |
		(eth_header.source_mac[3] << 8) | eth_header.source_mac[2];
	sg_write32(csr_reg_base, CDMA_CSR_150_Setting, val);
	printf("[CDMA_CSR_150_Setting] expected value:0x%x\n", val);
	read_val = sg_read32(csr_reg_base, CDMA_CSR_151_Setting);
	printf("[CDMA_CSR_150_Setting] real value:0x%x\n", read_val);

	vlan_header[0] = (eth_header.vlan_header.ether_type >> 8) & 0xff;
	vlan_header[1] = eth_header.vlan_header.ether_type & 0xff;
	vlan_header[2] = (eth_header.vlan_header.priority << 5 ) | 
			 (eth_header.vlan_header.priority << 4 ) |
			 ((eth_header.vlan_header.vlan_id >> 8) & 0xf);
	vlan_header[3] = (eth_header.vlan_header.vlan_id & 0xff);
	val = (vlan_header[3] << 24) | (vlan_header[2] << 16) |
		(vlan_header[1] << 8) | vlan_header[3];
	sg_write32(csr_reg_base, CDMA_CSR_151_Setting, val);
	printf("[CDMA_CSR_151_Setting] expected value:0x%x\n", val);
	read_val = sg_read32(csr_reg_base, CDMA_CSR_151_Setting);
	printf("[CDMA_CSR_151_Setting] real value:0x%x\n", read_val);
}
static void cdma_p2p_normal_csr_config(uintptr_t csr_reg_base){
	u32 reg_val = 0;

	/* enable replay and hardware replayfunction */
	reg_val = sg_read32(csr_reg_base, CDMA_CSR_0_OFFSET);
	reg_val |= ((1 << REG_CONNECTION_ON) | (1 << REG_HW_REPLAY_EN));
	sg_write32(csr_reg_base, CDMA_CSR_0_OFFSET,reg_val);
	/* same subinst replay max times*/
	reg_val = sg_read32(csr_reg_base, CDMA_CSR_4_OFFSET);
	reg_val &= ~(0xf << REG_REPLAY_MAX_TIME);
	reg_val |= (0xf << REG_REPLAY_MAX_TIME);
	sg_write32(csr_reg_base, CDMA_CSR_4_OFFSET,reg_val);
	/* when time cnt is bigger than this reg, trigger timeout replay*/
	reg_val = 0;
	reg_val |= 0xfffff;
	sg_write32(csr_reg_base, CDMA_CSR_5_OFFSET, reg_val);
	
	/* config  VLAN_ID */

	/* enable VLAN_ID */

	/* CRC Pad Control*/

	/* SA Insertion Control*/

	/* Checksum Insertion Control or TCP Payload Length*/

	/* sec_length*/
	reg_val = sg_read32(csr_reg_base, CDMA_CSR_4_OFFSET);
	reg_val &= ~(7 << REG_SEC_LENGTH);
	reg_val |= (5 << REG_SEC_LENGTH);
	sg_write32(csr_reg_base, CDMA_CSR_4_OFFSET, reg_val);
	/*when occur replay overflow, set this reg to recontinue*/

	/* IP header config*/
	eth_header_construction(csr_reg_base);
	/*config CDMA own tx/rx channel id and prio */
	reg_val = sg_read32(csr_reg_base, MTL_FAB_CSR_00);
	reg_val &= ~( 0xf << REG_CDMA_TX_CH_ID);
	reg_val |=  ( CDMA_TX_CHANNEL << REG_CDMA_TX_CH_ID);
	reg_val &= ~( 0xf << REG_CDMA_RX_CH_ID);
	reg_val |=  ( CDMA_RX_CHANNEL << REG_CDMA_RX_CH_ID);
	reg_val &= ~( 0x1 << REG_CDMA_FAB_BYPASS);
	reg_val &= ~( 0x3 << REG_CDMA_FAB_ARBITRATION);
	//reg_val |=  ( 0x1 << REG_CDMA_FAB_ARBITRATION);
	sg_write32(csr_reg_base, MTL_FAB_CSR_00, reg_val);
	/* WRITE c2c_rn */
	reg_val = sg_read32(csr_reg_base, CDMA_CSR_141_OFFSET);
	reg_val &= ~(0xff << REG_INTRA_DIE_WRITE_ADDR_H8);
	reg_val |= (0x80 << REG_INTRA_DIE_WRITE_ADDR_H8);
	sg_write32(csr_reg_base, CDMA_CSR_141_OFFSET, reg_val);
	/* READ c2c_rn */
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
	mmio_write_64(cmd_reg_base, low);
	//u64 low_val = mmio_read_64(cmd_reg_base);
	printf("function:%s, line:%d, low :0x%lx\n", __func__, __LINE__, low);
	//printf("function:%s, line:%d, low_val = 0x%lx\n", __func__, __LINE__, low_val);

	high = (data_length << CDMA_CMD_CMD_LENGTH) |
	       (src_addr_l32 << CDMA_CMD_SRC_START_ADDR_L32) ;
	mmio_write_64(cmd_reg_base + 0x8, high);
	//u64 high_val = mmio_read_64(cmd_reg_base + 0x8);
	printf("function:%s, line:%d, hign :0x%lx\n",__func__, __LINE__, high);
	//printf("function:%s, line:%d, hign_val = 0x%lx\n",__func__, __LINE__, high_val);

	low = (dst_addr_l32 << CDMA_CMD_DST_START_ADDR_L32);
	mmio_write_64(cmd_reg_base + 0x10, low);
	//low_val = mmio_read_64(cmd_reg_base + 0x10);
	printf("function:%s, line:%d, low :0x%lx\n", __func__, __LINE__, low);
	//printf("function:%s, line:%d, low_val = 0x%lx\n", __func__, __LINE__, low_val);

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

//#define DESC_MODE

#if defined(DESC_MODE)

static void cdma_p2p_desc_csr_config(uintptr_t csr_reg_base, u32 desc_addr_l32,u32 desc_addr_h1){
	u32 reg_val = 0;

	/*des axi max arlen*/
	reg_val = sg_read32(csr_reg_base, CDMA_CSR_4_OFFSET);
	reg_val |= (7 << REG_DES_MAX_ARLEN);
	sg_write32(csr_reg_base, CDMA_CSR_4_OFFSET, reg_val);
	/* L32 addr [38:7]*/
	sg_write32(csr_reg_base, CDMA_CSR_9_OFFSET, desc_addr_l32);
	/* h1 addr [39:39]*/
	sg_write32(csr_reg_base, CDMA_CSR_9_OFFSET, desc_addr_h1);
	/* reg_des_arcache  reg_des_arqos default*/

	/* desc net config*/
	reg_val = sg_read32(csr_reg_base, CDMA_CSR_142_OFFSET);
	reg_val &= ~(0xff << reg_des_read_addr_h8);
	reg_val |= (0x80 << reg_des_read_addr_h8);
	sg_write32(csr_reg_base, CDMA_CSR_142_OFFSET, reg_val);
}

int testcase_cdma_p2p_normal_desc(char* dma_base_mmap,
				  char* ddr_mapped_memory,
				  int desc_num,
				  u64 src_mem_start_addr,
				  u64 dst_mem_start_addr,
				  int src_format,
				  u32 data_length
				 )
{
	u32 reg_val = 0;
	u32 mem_tag = 0;
	u64 low = 0, high = 0;
	u32 count = 3000;
	u32 general_desc_phy_addr_l32;
	u32 general_desc_phy_addr_h1;
	u32 sys_desc_phy_addr_l32;
	u32 sys_desc_phy_addr_h1;
	u64 src_addr_h13 = ((src_mem_start_addr >> 32) | (mem_tag << 8)) & 0x1FFF;
	u64 src_addr_l32 = (src_mem_start_addr ) & 0xFFFFFFFF;
	u64 dst_addr_h13 = (dst_mem_start_addr >> 32) & 0x1FFF;
	u64 dst_addr_l32 = (dst_mem_start_addr ) & 0xFFFFFFFF;

	printf("src_mem_start_addr:0x%lx,src_addr_h13:0x%lx src_addr_l32 :0x%lx,\n\
		dst_mem_start_addr:0x%lx,dst_addr_h13:0x%lx,dst_addr_l32:0x%lx\n",\
		src_mem_start_addr,src_addr_h13,src_addr_l32,\
		dst_mem_start_addr,dst_addr_h13,dst_addr_l32);
	
	uintptr_t cmd_reg_base = (uintptr_t)(dma_base_mmap + CMD_REG_OFFSET);
	uintptr_t desc_updt = (uintptr_t)(dma_base_mmap + DESC_UPDT_OFFSET);
	uintptr_t csr_reg_base = (uintptr_t)(dma_base_mmap + CSR_REG_OFFSET);

	uintptr_t general_desc_virtual_addr = (uintptr_t)(ddr_mapped_memory + GENERAL_DESC_PHY_OFFSET);
	uintptr_t general_desc_phy_addr = (uintptr_t)(DDR_PHY_ADDRESS_START + GENERAL_DESC_PHY_OFFSET);
	dma_general_desc* dma_general_desc_addr = (dma_general_desc*)general_desc_virtual_addr;
	printf("cmd_reg_base:0x%lx, desc_updt:0x%lx, csr_reg_base :0x%lx general_desc_virtual_addr:0x%lx\n ", \
		cmd_reg_base, desc_updt, csr_reg_base, general_desc_virtual_addr);
	
	cdma_p2p_normal_csr_config(csr_reg_base);
	general_desc_phy_addr_l32 = ((general_desc_phy_addr)>> 7) & 0xFFFFFFFF;
	general_desc_phy_addr_h1 = ((general_desc_phy_addr)>> 39) & 0x1;
	
	/*At last,need add cdma_sys cmd */
	for(int i = 0;i < desc_num ;i++){
		dma_general_desc_addr +=  desc_num;

		dma_general_desc_addr->intr_en = 0;
		dma_general_desc_addr->cmd_type = (u32)CDMA_GENERAL;
		dma_general_desc_addr->src_data_format = src_format;
		dma_general_desc_addr->src_start_addr_h13 = src_addr_h13;
		dma_general_desc_addr->dst_start_addr_h13 = dst_addr_h13;
		dma_general_desc_addr->src_start_addr_l32 = src_addr_l32;
		dma_general_desc_addr->dst_start_addr_l32 = dst_addr_l32;
	} 
	dma_general_desc_addr = dma_general_desc_addr + 1;
	dma_sys_desc* dma_sys_desc_addr = (dma_sys_desc*)dma_general_desc_addr;
	dma_sys_desc_addr->intr_en = 1;
	dma_sys_desc_addr->cmd_type = (u32)CDMA_SYS;
	dma_sys_desc_addr->cmd_special_function = (u32)CHAIN_END;

	cdma_p2p_desc_csr_config(csr_reg_base, general_desc_phy_addr_l32, general_desc_phy_addr_h1);

	/* enable desc mode */
	reg_val = sg_read32(csr_reg_base, CDMA_CSR_0_OFFSET);
	reg_val |= (1 << REG_DES_MODE_ENABLE);
	sg_write32(csr_reg_base, CDMA_CSR_0_OFFSET,reg_val);

	/* After the desc command is executed, check whether the enable is lowered*/
	while(sg_read32(csr_reg_base, CDMA_CSR_0_OFFSET) & 0x1 != 0){
		usleep(1);
		if (--count == 0) {
			printf("[desc mode]cdma polling wait timeout\n");
			return -1;
		}
	}

	

}

#endif

int main()
{	
	size_t dma_length = CDMA_MEMORY_RANGE;
	off_t  dma_offset = CDMA_PHY_ADDRESS_START;
	size_t ddr_length = DDR_MEMORY_RANGE;
	off_t  ddr_offset = DDR_PHY_ADDRESS_START;
	int src_format;
	u32 data_length = 256;
	u64 dst_mem_start_addr = 0x2000;


	char* dma_mapped_memory = (char*)mmap_phy_addr(dma_length, dma_offset);
	char* ddr_mapped_memory = (char*)mmap_phy_addr(ddr_length, ddr_offset);
	
	uintptr_t ddr_value_virtual = (uintptr_t)(ddr_mapped_memory + DDR_PHY_OFFSET);
 	uintptr_t ddr_value_phy = DDR_PHY_ADDRESS_START + DDR_PHY_OFFSET;
	printf("ddr_value_virtual:0x%lx\n", ddr_value_virtual);
	
	for(int i = 0; i < 256 ; i++) {
		sg_write32(ddr_value_virtual, i * 4, i);
	}
	u32 ddr_value = sg_read32(ddr_value_virtual, 0x30);
	printf("ddr_value:%u\n", ddr_value);

  	testcase_cdma_p2p_normal_pio((char*)dma_mapped_memory,
				(u64)ddr_value_phy,
				(int)DATA_INT32,
				data_length,
				dst_mem_start_addr);

	munmap_dma_reg(dma_length, dma_mapped_memory);
	munmap_dma_reg(ddr_length, ddr_mapped_memory);

	return 0;
}
