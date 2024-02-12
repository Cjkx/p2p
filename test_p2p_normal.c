#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>

#include "cdma_p2p_common.h"
#include "cdma_p2p_normal.h"
#include "xlgmac_core.h"
#include "cdma_pmu.h"
#include "raw_tcp_ip.h"

#define USING_PMU

static u8 sec_length;

void set_normal_eth_header(eth_header_t *eth, const char *src_mac_str, const char *dest_mac_str) {
	parse_mac_address(src_mac_str, eth->source_mac);
	parse_mac_address(dest_mac_str, eth->dest_mac);
}
// ee:2d:a3:a0:60:17   6c:3c:8c:74:cc:c4
static void eth_header_construction(uintptr_t csr_reg_base) {
	u32 val;
	u8  vlan_header[4];
	u32 read_val;
	eth_header_t eth_header = {
		.dest_mac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
		.source_mac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
		.vlan_header = {
			.ether_type = 0x8100, 
			.priority = 7,
			.cfi = 0,
			.vlan_id = 4094,	
		}
	};

	set_normal_eth_header(&eth_header, "e2:e4:d7:5a:5e:84", "04:32:01:2c:ec:31") ;
	val = (eth_header.dest_mac[3] << 24) | (eth_header.dest_mac[2] << 16) |
	      (eth_header.dest_mac[1] << 8) | eth_header.dest_mac[0] ;
	sg_write32(csr_reg_base, CDMA_CSR_148_Setting, val);
	printf("[CDMA_CSR_148_Setting] expected value:0x%x\n", val);
	printf("[CDMA_CSR_148_Setting] real value:0x%x\n", 
	       sg_read32(csr_reg_base, CDMA_CSR_148_Setting));
	
	val = (eth_header.source_mac[1] << 24) | (eth_header.source_mac[0] << 16) |
		(eth_header.dest_mac[5] << 8) | eth_header.dest_mac[4];
	sg_write32(csr_reg_base, CDMA_CSR_149_Setting, val);
	printf("[CDMA_CSR_149_Setting] expected value:0x%x\n", val);
	printf("[CDMA_CSR_149_Setting] real value:0x%x\n", 
		sg_read32(csr_reg_base, CDMA_CSR_149_Setting));

	val = (eth_header.source_mac[5] << 24) | (eth_header.source_mac[4] << 16) |
		(eth_header.source_mac[3] << 8) | eth_header.source_mac[2];
	sg_write32(csr_reg_base, CDMA_CSR_150_Setting, val);
	printf("[CDMA_CSR_150_Setting] expected value:0x%x\n", val);
	printf("[CDMA_CSR_150_Setting] real value:0x%x\n", 
		sg_read32(csr_reg_base, CDMA_CSR_150_Setting));

	vlan_header[0] = (eth_header.vlan_header.ether_type >> 8) & 0xff;
	vlan_header[1] = eth_header.vlan_header.ether_type & 0xff;
	vlan_header[2] = (eth_header.vlan_header.priority << 5 ) | 
			 (eth_header.vlan_header.cfi << 4 ) |
			 ((eth_header.vlan_header.vlan_id >> 8) & 0xf);
	vlan_header[3] = (eth_header.vlan_header.vlan_id & 0xff);
	val = (vlan_header[3] << 24) | (vlan_header[2] << 16) |
	      (vlan_header[1] << 8) | vlan_header[0];
	sg_write32(csr_reg_base, CDMA_CSR_151_Setting, val);
	printf("[CDMA_CSR_151_Setting] expected value:0x%x\n", val);
	printf("[CDMA_CSR_151_Setting] real value:0x%x\n", 
		sg_read32(csr_reg_base, CDMA_CSR_151_Setting));
	
	val = sg_read32(csr_reg_base,CDMA_CSR_0_OFFSET);
	val |= (1 << REG_ETH_CSR_UPDT);
	sg_write32(csr_reg_base, CDMA_CSR_0_OFFSET, val);
	printf("[CDMA_CSR_0_OFFSET]:0x%x\n", val);
}

static void cdma_p2p_normal_csr_config(uintptr_t csr_reg_base){
	u32 reg_val = 0;

	/* enable replay and hardware replayfunction */
	reg_val = sg_read32(csr_reg_base, CDMA_CSR_0_OFFSET);
	reg_val |= ((1 << REG_CONNECTION_ON) | (1 << REG_HW_REPLAY_EN));
	sg_write32(csr_reg_base, CDMA_CSR_0_OFFSET,reg_val);
	printf("[CDMA_CSR_0_OFFSET]:0x%x\n", reg_val);
	/* same subinst replay max times*/
	// reg_val = sg_read32(csr_reg_base, CDMA_CSR_4_OFFSET);
	// reg_val &= ~(0xf << REG_REPLAY_MAX_TIME);
	// reg_val |= (0xf << REG_REPLAY_MAX_TIME);
	// sg_write32(csr_reg_base, CDMA_CSR_4_OFFSET,reg_val);
	// printf("[CDMA_CSR_4_OFFSET]:0x%x\n", reg_val);

	/* when time cnt is bigger than this reg, trigger timeout replay*/
	// reg_val = 0;
	// reg_val |= 0x19000;
	// sg_write32(csr_reg_base, CDMA_CSR_5_OFFSET, reg_val);
	// printf("[CDMA_CSR_5_OFFSET]:0x%x\n", reg_val);
	/* config  VLAN_ID */

	/* enable VLAN_ID */

	/* CRC Pad Control*/

	/* SA Insertion Control*/

	/* Checksum Insertion Control or TCP Payload Length*/

	/* sec_length*/
	reg_val = sg_read32(csr_reg_base, CDMA_CSR_4_OFFSET);
	reg_val &= ~(7 << REG_SEC_LENGTH);
	reg_val |= (sec_length << REG_SEC_LENGTH);
	sg_write32(csr_reg_base, CDMA_CSR_4_OFFSET, reg_val);
	printf("[CDMA_CSR_4_OFFSET]:0x%x\n", reg_val);
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
	printf("[MTL_FAB_CSR_00]:0x%x\n", reg_val);
	/* WRITE c2c_rn  / READ c2c_rn*/
	reg_val = sg_read32(csr_reg_base, CDMA_CSR_141_OFFSET);
	reg_val &= ~(0xff << REG_INTRA_DIE_WRITE_ADDR_H8);
	reg_val |= (0x80 << REG_INTRA_DIE_WRITE_ADDR_H8);
	reg_val &= ~(0xff << REG_INTRA_DIE_READ_ADDR_H8);
	reg_val |= (0x80 << REG_INTRA_DIE_READ_ADDR_H8);
	sg_write32(csr_reg_base, CDMA_CSR_141_OFFSET, reg_val);
	printf("[CDMA_CSR_141_OFFSET]:0x%x\n", reg_val);

	sg_write32(csr_reg_base, NORMAL_FRAME_LEN, 0x200000);
	printf("[NORMAL_FRAME_LEN]:0x%x\n", sg_read32(csr_reg_base, NORMAL_FRAME_LEN));

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
	u64 val = 0;
	u64 src_addr_h13 = ((src_mem_start_addr >> 32) | (mem_tag << 8)) & 0x1FFF;
	u64 src_addr_l32 = (src_mem_start_addr ) & 0xffffffff;
	u64 dst_addr_h13 = ((dst_mem_start_addr >> 32) | (mem_tag << 8)) & 0x1FFF;
	u64 dst_addr_l32 = (dst_mem_start_addr ) & 0xffffffff;
	u32 count = 1500;

	debug_log("src_mem_start_addr:0x%lx src_addr_h13:0x%lx src_addr_l32 :0x%lx\n",
	 	src_mem_start_addr, src_addr_h13, src_addr_l32);
	debug_log("dst_mem_start_addr:0x%lx dst_addr_h13:0x%lx dst_addr_l32:0x%lx\n",
	 	dst_mem_start_addr, dst_addr_h13, dst_addr_l32);
	
	uintptr_t cmd_reg_base = (uintptr_t)(dma_base_mmap + CMD_REG_OFFSET);
	uintptr_t desc_updt = (uintptr_t)(dma_base_mmap + DESC_UPDT_OFFSET);
	uintptr_t csr_reg_base = (uintptr_t)(dma_base_mmap + CSR_REG_OFFSET);
	debug_log("cmd_reg_base:0x%lx, desc_updt:0x%lx, csr_reg_base :0x%lx\n ", \
		cmd_reg_base, desc_updt, csr_reg_base);

	cdma_p2p_normal_csr_config(csr_reg_base);

	val = (1ull << CDMA_CMD_INTR_ENABLE) |
	      ((u64)CDMA_GENERAL << CDMA_CMD_CMD_TYPE) |
	      ((u64)src_format << CDMA_CMD_SRC_DATA_FORMAT) |
	      (src_addr_h13 << CDMA_CMD_SRC_START_ADDR_H13) |
	      (dst_addr_h13 << CDMA_CMD_DST_START_ADDR_H13);
	mmio_write_64(cmd_reg_base, val);
	debug_log("[CDMA_CMD_0]:0x%lx\n", val);
	
	val = (data_length << CDMA_CMD_CMD_LENGTH) |
	       (src_addr_l32 << CDMA_CMD_SRC_START_ADDR_L32) ;
	mmio_write_64(cmd_reg_base + 0x8, val);
	debug_log("[CDMA_CMD_1]:0x%lx\n", val);
	

	val = (dst_addr_l32 << CDMA_CMD_DST_START_ADDR_L32);
	mmio_write_64(cmd_reg_base + 0x10, val);
	debug_log("[CDMA_CMD_2] low :0x%lx\n", val);

	/* In pio mode write 1 to update des*/
	sg_write32(desc_updt, REG_DESCRIPTOR_UPDATE, 1);
	debug_log("write CDMA_UPDT_DES_UPDT to 0x1\n");

	/* poll */
	while ((sg_read32(csr_reg_base,CDMA_CSR_20_Setting) & 0x1) != 0x1) {
		usleep(1);
		if (--count == 0) {
			printf("cdma polling wait timeout\n");
			return -1;
		}
		if (count % 100 == 0){
			printf("cdma transfer consumes %d us\n", 1500 - count);
		}
	}
	printf("cdma transfer consumes %d us\n", 1500 - count);
	// clear intr
	val = sg_read32(csr_reg_base,CDMA_CSR_20_Setting);
	printf("[INTR] val:0x%lx", val);
	val = 0;
	val |= (1 << INTR_CMD_DONE_STATUS);
	sg_write32(csr_reg_base, CDMA_CSR_20_Setting, val);
	printf("CDMA polling complete\n");

	return 0;
}

#define DESC_MODE

#if defined(DESC_MODE)

static void cdma_p2p_desc_csr_config(uintptr_t csr_reg_base) {
	u32 reg_val = 0;
	u32 general_desc_phy_addr_l32;
	u32 general_desc_phy_addr_h1;

	uintptr_t general_desc_phy_addr = (uintptr_t)(DDR_PHY_ADDRESS_START + GENERAL_DESC_PHY_OFFSET);
	general_desc_phy_addr_l32 = ((general_desc_phy_addr)>> 7) & 0xFFFFFFFF;
	general_desc_phy_addr_h1 = ((general_desc_phy_addr)>> 39) & 0x1;
	printf("[DESC MODE] general_desc_phy_addr:0x%lx general_desc_phy_addr_l32:0x%x\
	        general_desc_phy_addr_h1:0x%x \n",
		general_desc_phy_addr, general_desc_phy_addr_l32, general_desc_phy_addr_h1);

	/* L32 addr [38:7]*/
	sg_write32(csr_reg_base, CDMA_CSR_9_OFFSET, general_desc_phy_addr_l32);
	printf("[DESC MODE][CDMA_CSR_9_OFFSET]:get 0x%x\n",sg_read32(csr_reg_base,CDMA_CSR_9_OFFSET));

	/* h1 addr [39:39]*/
	sg_write32(csr_reg_base, CDMA_CSR_10_OFFSET, general_desc_phy_addr_h1);
	printf("[DESC MODE][CDMA_CSR_10_OFFSET]:get 0x%x\n",sg_read32(csr_reg_base,CDMA_CSR_10_OFFSET));

	/* desc net config*/
	reg_val = sg_read32(csr_reg_base, CDMA_CSR_142_OFFSET);
	reg_val &= ~(0xff << REG_DES_READ_ADDR_H8);
	reg_val |= (0x80 << REG_DES_READ_ADDR_H8);
	sg_write32(csr_reg_base, CDMA_CSR_142_OFFSET, reg_val);
	printf("[DESC MODE][CDMA_CSR_142_OFFSET]:get 0x%x\n",sg_read32(csr_reg_base,CDMA_CSR_142_OFFSET));
}

static void cdma_p2p_general_cmd_config(uintptr_t cmd_reg_base,
				     u64 src_mem_start_addr,
				     u64 dst_mem_start_addr,
				     int src_format,
				     u32 data_length){
	u64 reg_val;
	u32 mem_tag = 0;
	u64 src_addr_h13 = ((src_mem_start_addr >> 32) | (mem_tag << 8)) & 0x1FFF;
	u64 src_addr_l32 = (src_mem_start_addr ) & 0xFFFFFFFF;
	u64 dst_addr_h13 = ((dst_mem_start_addr >> 32) | (mem_tag << 8)) & 0x1FFF;
	u64 dst_addr_l32 = (dst_mem_start_addr ) & 0xFFFFFFFF;

	printf("[DESC MODE] src_mem_start_addr:0x%lx,src_addr_h13:0x%lx src_addr_l32 :0x%lx,\n\
		[DESC MODE] dst_mem_start_addr:0x%lx,dst_addr_h13:0x%lx,dst_addr_l32:0x%lx\n",\
		src_mem_start_addr,src_addr_h13,src_addr_l32,\
		dst_mem_start_addr,dst_addr_h13,dst_addr_l32);

	reg_val = 0;
	reg_val = (0ull << CDMA_CMD_INTR_ENABLE) |
	      ((u64)CDMA_GENERAL << CDMA_CMD_CMD_TYPE) |
	      ((u64)src_format << CDMA_CMD_SRC_DATA_FORMAT) |
	      (src_addr_h13 << CDMA_CMD_SRC_START_ADDR_H13) |
	      (dst_addr_h13 << CDMA_CMD_DST_START_ADDR_H13);
	mmio_write_64(cmd_reg_base, reg_val);
	printf("[CDMA_CMD_0]:0x%lx\n", reg_val);
	
	reg_val = 0;
	reg_val = (data_length << CDMA_CMD_CMD_LENGTH) |
	       (src_addr_l32 << CDMA_CMD_SRC_START_ADDR_L32) ;
	mmio_write_64(cmd_reg_base + 0x8, reg_val);
	printf("[CDMA_CMD_1]:0x%lx\n", reg_val);
	
	reg_val = 0;
	reg_val = (dst_addr_l32 << CDMA_CMD_DST_START_ADDR_L32);
	mmio_write_64(cmd_reg_base + 0x10, reg_val);
	printf("[CDMA_CMD_2] low :0x%lx\n", reg_val);

}



int testcase_cdma_p2p_normal_desc(char* dma_base_mmap,
				  char* ddr_mapped_memory,
				  u8 desc_num,
				  u64 src_mem_start_addr,
				  u64 dst_mem_start_addr,
				  int src_format,
				  u32 data_length
				 )
{
	u32 reg_val = 0;
	int ret = 0;
	u32 data_size;
	uintptr_t csr_reg_base = (uintptr_t)(dma_base_mmap + CSR_REG_OFFSET);
	uintptr_t general_desc_virtual_addr = (uintptr_t)(ddr_mapped_memory + GENERAL_DESC_PHY_OFFSET);
	
	printf("[NORMAL DESC MODE] csr_reg_base :0x%lx general_desc_virtual_addr:0x%lx\n ",
		 csr_reg_base, general_desc_virtual_addr);
	data_size = 4;
	cdma_p2p_normal_csr_config(csr_reg_base);
	cdma_p2p_desc_csr_config(csr_reg_base);
	for(int i = 0; i < desc_num; i++) {
		cdma_p2p_general_cmd_config(general_desc_virtual_addr, src_mem_start_addr, dst_mem_start_addr,
				    src_format, data_length);
		if(i == desc_num - 1)
			break;
		general_desc_virtual_addr += GENERAL_CMD_SIZE;
		src_mem_start_addr += data_length * data_size;
		dst_mem_start_addr += data_length * data_size;
	}
	p2p_sys_end_chain_config((uintptr_t)(general_desc_virtual_addr + GENERAL_CMD_SIZE));

	p2p_enable_desc(csr_reg_base, NORMAL_GENERAL_DESC);

	ret = p2p_poll(csr_reg_base, NORMAL_GENERAL_DESC);

	return ret;
}
#endif

int main(int argc, char *argv[])
{	
	size_t dma_length = CDMA_MEMORY_RANGE;
	off_t  dma_offset = CDMA_PHY_ADDRESS_START;
	size_t ddr_length = DDR_MEMORY_RANGE;
	off_t  ddr_offset = DDR_PHY_ADDRESS_START;
	size_t xlgmac_length = XLGMAC_MEMORY_RANGE;
	off_t  xlgmac_offset = XLGMAC_ETH_BASE_START;
	char* endptr;
	u8 desc_num;
	u8 filter_index = 0;
	int src_format;
	int ret;

	if (argc > 7 || argc < 4 ) {
        	printf("Usage: test_cdma_p2p src_addr dst_addr size sec_len pio/desc [desc_num] \n");
        	return -1;
    	}

	char* dma_mapped_memory = (char*)mmap_phy_addr(dma_length, dma_offset);
	char* ddr_mapped_memory = (char*)mmap_phy_addr(ddr_length, ddr_offset);
	char* xlgmac_mapped_memory = (char*)mmap_phy_addr(xlgmac_length, xlgmac_offset);
	
	printf("mmap success!\n");

	xlgmac_set_rx_crc_strip(xlgmac_mapped_memory);
	// xlgmac_set_loopback_mode(xlgmac_mapped_memory, 1);

	uintptr_t src_mem_start_addr = (uintptr_t)strtol(argv[1],&endptr,16);
	u64 dst_mem_start_addr = (u64)strtol(argv[2],&endptr,16);
	u32 data_length = (u32)atoi(argv[3]);
	sec_length = (u8)atoi(argv[4]);
	printf("src_mem_start_addr:0x%lx dst_mem_start_addr:0x%lx data_length %d \n",
		src_mem_start_addr, dst_mem_start_addr, data_length);
	
	uintptr_t src_ddr_addr_virtual = (uintptr_t)(ddr_mapped_memory + src_mem_start_addr);
	uintptr_t dst_ddr_value_virtual = (uintptr_t)(ddr_mapped_memory + dst_mem_start_addr);
	
#ifdef USING_PMU

	enable_cdma_perf_monitor((char*)dma_mapped_memory, 
				 (char*)ddr_mapped_memory, 
				 CDMA_PMU_START_ADDR, 
				 CDMA_PMU_SIZE);

#endif
	if ((!strcmp(argv[5], "pio")) && (argc == 6)) {
		printf("[testcase:P2P NORMAL PIO] PIO_num:%d\n", 1);
		write_data_32(src_ddr_addr_virtual, dst_ddr_value_virtual, data_length);
		ret = testcase_cdma_p2p_normal_pio((char*)dma_mapped_memory,
						   (u64)src_mem_start_addr,
						   (int)DATA_INT32,
						   data_length,
						   dst_mem_start_addr);
	} else if ((!strcmp(argv[5], "desc")) && (argc == 7)) {
		desc_num = (u8)atoi(argv[6]);
		write_data_32(src_ddr_addr_virtual, dst_ddr_value_virtual, data_length * desc_num);
		printf("[testcase:P2P NORMAL DESC] desc_num:%d\n", desc_num);
		ret = testcase_cdma_p2p_normal_desc((char*)dma_mapped_memory,
				 		    (char*)ddr_mapped_memory,
				 		    desc_num,
				                    (u64)src_mem_start_addr,
				                    (u64)dst_mem_start_addr,
				                    (int)DATA_INT32,
				                     data_length);
	} else {
		printf("Usage: test_cdma_p2p src_addr dst_addr size pio/desc [desc_num] \n");
		goto munmap;
	}

	
  	
	ret = data_compare(src_ddr_addr_virtual, dst_ddr_value_virtual, 
			  dst_mem_start_addr, data_length);

#ifdef USING_PMU
	usleep(10);
	disable_cdma_perf_monitor((char*)dma_mapped_memory);
	show_cdma_pmu_data( (char*)ddr_mapped_memory, CDMA_PMU_START_ADDR, 0);
#endif

#if 0
	if ((argc == 7 && !strcmp(argv[4], "pio") &&  !strcmp(argv[6], "r")) || 
	    (argc == 6 && !strcmp(argv[4], "desc") && !strcmp(argv[5], "r"))) {
		
	}
#endif

munmap:
	munmap_dma_reg(dma_length, dma_mapped_memory);
	munmap_dma_reg(ddr_length, ddr_mapped_memory);
	munmap_dma_reg(xlgmac_length, xlgmac_mapped_memory);
	
	return 0;
}
