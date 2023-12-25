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

#define eth_header_len 16

//6e:40:68:7f:97:34
static void Constructe_Ethernet_Header(uintptr_t src_ddr_addr_virtual) {
	eth_header_t eth_header = {
		.dest_mac = {0x6c, 0x3c, 0x8c, 0x74, 0xcc, 0xc4},
		.source_mac = {0x6e, 0x40, 0x68, 0x7f, 0x97, 0x34},
		.vlan_header = {
			.ether_type = 0x8100, 
			.priority = 0,
			.cfi = 0,
			.vlan_id = 0,	
		}
	};

	for (int i = 0; i < 6; ++i) {
             sg_write8(src_ddr_addr_virtual,i ,eth_header.dest_mac[i]); 
        }

	for (int i = 0; i < 6; ++i) {
             sg_write8(src_ddr_addr_virtual,i + 6 ,eth_header.source_mac[i]); 
        }

	u8 temp = (eth_header.vlan_header.priority << 5 | 
	           eth_header.vlan_header.cfi << 4 |
		   eth_header.vlan_header.vlan_id & 0xf);
	
	sg_write8(src_ddr_addr_virtual, 12, (eth_header.vlan_header.ether_type >> 8) & 0xff);
	sg_write8(src_ddr_addr_virtual, 13, (eth_header.vlan_header.ether_type ) & 0xff);
	sg_write8(src_ddr_addr_virtual, 14,  temp);
	sg_write8(src_ddr_addr_virtual, 15, (eth_header.vlan_header.vlan_id  ) & 0xff);
	
	dump((void*)src_ddr_addr_virtual,eth_header_len);
	printf("\n");
	printf("\n");
}

static void p2p_tcp_csr_config(uintptr_t csr_reg_base, u8 mode){
	u32 reg_val = 0;

	/* enable replay and hardware replayfunction */
	reg_val = sg_read32(csr_reg_base, CDMA_CSR_0_OFFSET);
	reg_val &= ~((1 << REG_CONNECTION_ON) | (1 << REG_HW_REPLAY_EN));
	reg_val |= ((1 << REG_CONNECTION_ON) | (0 << REG_HW_REPLAY_EN));
	sg_write32(csr_reg_base, CDMA_CSR_0_OFFSET,reg_val);
	printf("[CDMA_CSR_0_OFFSET]:0x%x\n", reg_val);
	if (mode == TCP_SEND) {
		/* tcp_csr_updt */
		reg_val = sg_read32(csr_reg_base, TCP_CSR_20_Setting);
		reg_val |= (1 << REG_TCP_CSR_UPDT);
		sg_write32(csr_reg_base, TCP_CSR_20_Setting,reg_val);
		printf("[TCP_CSR_20_Setting]:0x%x\n", reg_val);
	}
	
	/*config CDMA own tx/rx channel id and prio */
	reg_val = sg_read32(csr_reg_base, MTL_FAB_CSR_00);
	reg_val &= ~( 0xf << REG_CDMA_TX_CH_ID);
	reg_val |=  ( CDMA_TX_CHANNEL << REG_CDMA_TX_CH_ID);
	reg_val &= ~( 0xf << REG_CDMA_RX_CH_ID);
	reg_val |=  ( CDMA_RX_CHANNEL << REG_CDMA_RX_CH_ID);
	reg_val &= ~( 0x1 << REG_CDMA_FAB_BYPASS);
	reg_val &= ~( 0x3 << REG_CDMA_FAB_ARBITRATION);
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

}

static void p2p_tcp_desc_csr_config(uintptr_t csr_reg_base, 
					 u64 tcp_des_addr,
					 u64 tcp_des_last_addr,
					 u8 mode) {
	u32 reg_val = 0;
	u64 tcp_des_addr_l32;
	u64 tcp_des_addr_h4;
	u64 tcp_des_last_addr_l32;
	u64 tcp_des_last_addr_h4;

	tcp_des_addr_l32 = (tcp_des_addr >> 4) & 0xffffffff;
	tcp_des_addr_h4 = (tcp_des_addr >> 36) & 0xf;
	tcp_des_last_addr_l32 = (tcp_des_last_addr >> 4) & 0xffffffff;
	tcp_des_last_addr_h4 = (tcp_des_last_addr >> 36) & 0xf;
	printf("tcp_des_addr:0x%lx tcp_des_last_addr:0x%lx\n",
		tcp_des_addr, tcp_des_last_addr);

	if(TCP_SEND == mode) {
		sg_write32(csr_reg_base, TCP_CSR_03_Setting, tcp_des_addr_l32);
		sg_write32(csr_reg_base, TCP_CSR_04_Setting, tcp_des_addr_h4);
		sg_write32(csr_reg_base, TCP_CSR_07_Setting, tcp_des_last_addr_l32);
		sg_write32(csr_reg_base, TCP_CSR_08_Setting, tcp_des_last_addr_h4);
		printf("tcp_des_addr_l32 :0x%x\n", sg_read32(csr_reg_base, TCP_CSR_03_Setting));
		printf("tcp_des_addr_h4 :0x%x\n", sg_read32(csr_reg_base, TCP_CSR_04_Setting));
		printf("tcp_des_last_addr_l32 :0x%x\n", sg_read32(csr_reg_base, TCP_CSR_07_Setting));
		printf("tcp_des_last_addr_h4 :0x%x\n", sg_read32(csr_reg_base, TCP_CSR_08_Setting));
		reg_val = sg_read32(csr_reg_base, CDMA_CSR_143_Setting);
		reg_val &= ~(0xff << REG_TXCH_READ_ADDR_H8);
		reg_val |= (0x80 << REG_TXCH_READ_ADDR_H8);
		reg_val &= ~(0xff << REG_TXCH_WRITE_ADDR_H8);
		reg_val |= (0x80 << REG_TXCH_WRITE_ADDR_H8);
		sg_write32(csr_reg_base, CDMA_CSR_143_Setting, reg_val);
		printf("[DESC MODE][CDMA_CSR_143_OFFSET]:get 0x%x\n",
			sg_read32(csr_reg_base, CDMA_CSR_143_Setting));
	} else if(TCP_RECEIVE == mode) {
		sg_write32(csr_reg_base, TCP_CSR_05_Setting, tcp_des_addr_l32);
		sg_write32(csr_reg_base, TCP_CSR_06_Setting, tcp_des_addr_h4);
		sg_write32(csr_reg_base, TCP_CSR_09_Setting, tcp_des_last_addr_l32);
		sg_write32(csr_reg_base, TCP_CSR_10_Setting, tcp_des_last_addr_h4);
		printf("tcp_des_addr_l32 :0x%x\n", sg_read32(csr_reg_base, TCP_CSR_05_Setting));
		printf("tcp_des_addr_h4 :0x%x\n", sg_read32(csr_reg_base, TCP_CSR_06_Setting));
		printf("tcp_des_last_addr_l32 :0x%x\n", sg_read32(csr_reg_base, TCP_CSR_09_Setting));
		printf("tcp_des_last_addr_h4 :0x%x\n", sg_read32(csr_reg_base, TCP_CSR_10_Setting));
		reg_val = sg_read32(csr_reg_base, CDMA_CSR_144_Setting);
		reg_val &= ~(0xff << REG_RXCH_READ_ADDR_H8);
		reg_val |= (0x80 << REG_RXCH_READ_ADDR_H8);
		reg_val &= ~(0xff << REG_RXCH_WRITE_ADDR_H8);
		reg_val |= (0x80 << REG_RXCH_WRITE_ADDR_H8);
		sg_write32(csr_reg_base, CDMA_CSR_144_Setting, reg_val);
		printf("[DESC MODE][CDMA_CSR_144_OFFSET]:get 0x%x\n",
			sg_read32(csr_reg_base, CDMA_CSR_144_Setting));
	}
}

static uintptr_t p2p_tcp_send_cmd_config(uintptr_t cmd_reg_base, 
				 	 u64 src_mem_start_addr, 
				    	 u32 data_length, 
				   	 u64 frame_length,
				   	 u32 cmd_id,
				   	 u8 desc_flag) {
	u64 reg_val;
	u32 mem_tag = 0;
	u8 fd_flag;
	u8 ld_flag;
	u64 src_addr_h8 =  (src_mem_start_addr >> 32) & 0xff;
	u64 src_addr_l32 = (src_mem_start_addr ) & 0xFFFFFFFF;

	printf("[TCP SEND MODE] src_mem_start_addr:0x%lx,src_addr_h8:0x%lx src_addr_l32 :0x%lx\n",
		src_mem_start_addr, src_addr_h8, src_addr_l32);


	switch (desc_flag)
	{
		case FD:
			fd_flag = 1;
			ld_flag = 0;
			break;
		case LD:
			fd_flag = 0;
			ld_flag = 1;
			frame_length = 0;
			break;
		case MD:
			fd_flag = 0;
			ld_flag = 0;
			frame_length = 0;
			break;
		case FLD:
			fd_flag = 1;
			ld_flag = 1;
			break;
		default:
			printf("choose desc_flag error!");
			return -1;
			break;
	}

	reg_val =0;
	reg_val = ((1ull << CDMA_CMD_INTR_ENABLE) |
		   (1ull << CMD_TCP_SEND_OWN) |
		   ((u64)fd_flag << CMD_TCP_SEND_FD) |
		   ((u64)ld_flag << CMD_TCP_SEND_LD) |
		   ((u64)CDMA_TCP_SEND << CMD_TCP_SEND_CMD_TYPE) |
		   ((u64)data_length << CMD_TCP_SEND_BUF_LEN) |
		   ((u64)frame_length << CMD_TCP_SEND_FRAME_LEN));
	mmio_write_64(cmd_reg_base, reg_val);
	printf("[CDMA_CMD_0]:0x%lx\n", reg_val);

	reg_val =0;
	reg_val = ((src_addr_l32 << CMD_TCP_SEND_BUF_ADDR_L32) |
		   (src_addr_h8 << CMD_TCP_SEND_BUF_ADDR_H8) |
		   ((u64)cmd_id << CMD_TCP_SEND_CMD_ID));
	mmio_write_64(cmd_reg_base + 0x8, reg_val);
	printf("[CDMA_CMD_1]:0x%lx\n", reg_val);

}

static uintptr_t p2p_tcp_rcv_cmd_config (uintptr_t cmd_reg_base, 
				 	 u64 dst_mem_start_addr, 
				    	 u32 data_length, 
				   	 u32 cmd_id) {
	u64 reg_val;
	u64 dst_addr_h8 =  (dst_mem_start_addr >> 32) & 0xff;
	u64 dst_addr_l32 = (dst_mem_start_addr ) & 0xFFFFFFFF;

	printf("[TCP RCV MODE] dst_mem_start_addr:0x%lx,dst_addr_h8:0x%lx dst_addr_l32 :0x%lx\n",
		dst_mem_start_addr, dst_addr_h8, dst_addr_l32);

	reg_val =0;
	reg_val = ((1ull << CMD_TCP_RCV_INTR_ENABLE) |
		   (1ull << CMD_TCP_RCV_OWN) |
		   ((u64)CDMA_TCP_RECEIVE << CMD_TCP_RCV_CMD_TYPE) |
		   ((u64)data_length << CMD_TCP_RCV_BUF_LEN));
	mmio_write_64(cmd_reg_base, reg_val);
	printf("[CDMA_CMD_0]:0x%lx\n", reg_val);

	reg_val =0;
	reg_val = ((dst_addr_l32 << CMD_TCP_RCV_BUF_ADDR_L32) |
		   (dst_addr_h8 << CMD_TCP_RCV_BUF_ADDR_H8) |
		   ((u64)cmd_id << CMD_TCP_RCV_CMD_ID));
	mmio_write_64(cmd_reg_base + 0x8, reg_val);
	printf("[CDMA_CMD_1]:0x%lx\n", reg_val);

}


int testcase_p2p_tcp_send_mode (char* dma_base_mmap,
			   	char* ddr_mapped_memory,
			  	u64 src_mem_start_addr,
			  	u32 data_length,
			   	u32 frame_length) {
	u32 reg_val = 0;
	u32 count = 1500;
	u32 cmd_id = 0;
	u8 desc_flag;
	u32 desc_num = frame_length / data_length;
	uintptr_t desc_current_virtual_addr;
	uintptr_t desc_phy_eaddr;

	uintptr_t csr_reg_base = (uintptr_t)(dma_base_mmap + CSR_REG_OFFSET);
	uintptr_t desc_virtual_saddr = (uintptr_t)(ddr_mapped_memory + TCP_SEND_DESC_PHY_OFFSET);
	uintptr_t desc_phy_saddr = (uintptr_t)(DDR_PHY_ADDRESS_START + TCP_SEND_DESC_PHY_OFFSET);

	printf("[TCP SEND MODE] csr_reg_base :0x%lx desc_virtual_saddr:0x%lx\n ",
		csr_reg_base, desc_virtual_saddr);
	
	p2p_tcp_csr_config(csr_reg_base, TCP_SEND);

	/* send */
	desc_current_virtual_addr = desc_virtual_saddr;


	for(int i = 0; i < desc_num; i++) {
		if(desc_num == 1){
			desc_flag = FLD;
		} else if(desc_num == 2) {
			if(i == 0)
				desc_flag = FD;
			else if (i == 1)
				desc_flag = LD;
		}else if(desc_num > 2) {
			if(i == 0)
				desc_flag = FD;
			else if(i == desc_num -1)
				desc_flag = LD;
			else
				desc_flag = MD;
		}
		p2p_tcp_send_cmd_config(desc_current_virtual_addr, src_mem_start_addr, data_length,
					frame_length, cmd_id, desc_flag);
		desc_current_virtual_addr += TCP_RCV_CMD_SIZE;
		src_mem_start_addr += data_length;
		cmd_id++;
	}

		
	desc_phy_eaddr = desc_phy_saddr + desc_num * TCP_SEND_CMD_SIZE;
	p2p_tcp_desc_csr_config(csr_reg_base, desc_phy_saddr, desc_phy_eaddr, TCP_SEND);
	
	p2p_enable_desc(csr_reg_base, TCP_SEND);

	p2p_poll(csr_reg_base, TCP_SEND);

	//clear intr
	reg_val = sg_read32(csr_reg_base, CDMA_CSR_20_Setting);
	reg_val |= (1 << INTR_TCP_SEND_CMD_DONE);
	sg_write32(csr_reg_base, CDMA_CSR_20_Setting, reg_val);
	printf("[TCP SEND MODE] CDMA polling complete\n");
}

int testcase_p2p_tcp_rcv_mode(char* dma_base_mmap,
			      char* ddr_mapped_memory,
			      u64 dst_mem_start_addr,
			      u32 data_length,
			      u32 desc_num) {
	u32 reg_val = 0;
	u32 count = 1500;
	u32 crc_len = 0;
	u32 cmd_id = 0;
	u32 cmd_id_send;
	u32 cmd_id_rcv;
	uintptr_t desc_current_virtual_addr;
	uintptr_t desc_phy_eaddr;

	uintptr_t csr_reg_base = (uintptr_t)(dma_base_mmap + CSR_REG_OFFSET);
	uintptr_t desc_virtual_saddr = (uintptr_t)(ddr_mapped_memory + TCP_RCV_DESC_PHY_OFFSET);
	uintptr_t desc_phy_saddr = (uintptr_t)(DDR_PHY_ADDRESS_START + TCP_RCV_DESC_PHY_OFFSET);

	printf("[TCP REV MODE] desc_phy_saddr :0x%lx desc_virtual_saddr:0x%lx\n ",
		desc_phy_saddr, desc_virtual_saddr);
	
	p2p_tcp_csr_config(csr_reg_base, TCP_RECEIVE);

	desc_current_virtual_addr = desc_virtual_saddr;
	for(int i = 0; i < desc_num; i++) {
		if(i == desc_num - 1){
			crc_len = 4;
		} else
			crc_len = 0;
		p2p_tcp_rcv_cmd_config(desc_current_virtual_addr, dst_mem_start_addr, data_length + crc_len, cmd_id++);
		desc_current_virtual_addr += TCP_RCV_CMD_SIZE;
		dst_mem_start_addr += data_length;
	}
	
	desc_phy_eaddr = desc_phy_saddr + desc_num * TCP_RCV_CMD_SIZE;

	p2p_tcp_desc_csr_config(csr_reg_base, desc_phy_saddr, desc_phy_eaddr, TCP_RECEIVE);

	p2p_enable_desc(csr_reg_base, TCP_RECEIVE);

	p2p_poll(csr_reg_base, TCP_RECEIVE);
	// clear intr
	reg_val = sg_read32(csr_reg_base, CDMA_CSR_20_Setting);
	reg_val |= (1 << INTR_TCP_RCV_CMD_DONE);
	sg_write32(csr_reg_base, CDMA_CSR_20_Setting, reg_val);
	printf("[TCP RCV MODE] CDMA polling complete\n");
	
	cmd_id_send = sg_read32(csr_reg_base, TCP_CSR_14_Setting);
	cmd_id_rcv = sg_read32(csr_reg_base, TCP_CSR_13_Setting);
	if(cmd_id_send == cmd_id_rcv){
		printf("[TCP RCV MODE] cmd_id_send:%d, cmd_id_rcv:%d, data rcv complete! \n",
			cmd_id_send,cmd_id_rcv);
	} else {
		printf("[TCP RCV MODE] cmd_id_send:%d, cmd_id_rcv:%d, data rcv fail! \n",
			cmd_id_send,cmd_id_rcv);
	}
}

static void rcv_poll(char* dma_base_mmap) {
	uintptr_t csr_reg_base = (uintptr_t)(dma_base_mmap + CSR_REG_OFFSET);
	u32 reg_val = 0;

	p2p_poll(csr_reg_base, TCP_RECEIVE);
	// clear intr
	reg_val = sg_read32(csr_reg_base, CDMA_CSR_20_Setting);
	reg_val |= (1 << INTR_TCP_RCV_CMD_DONE);
	sg_write32(csr_reg_base, CDMA_CSR_20_Setting, reg_val);
	printf("[TCP RCV MODE] CDMA polling complete\n");
}

int main(int argc, char *argv[])
{	
	size_t dma_length = CDMA_MEMORY_RANGE;
	off_t  dma_offset = CDMA_PHY_ADDRESS_START;
	size_t ddr_length = DDR_MEMORY_RANGE;
	off_t  ddr_offset = DDR_PHY_ADDRESS_START;
	size_t xlgmac_length = XLGMAC_MEMORY_RANGE;
	off_t  xlgmac_offset = XLGMAC_ETH_BASE_START;
	char* endptr;
	int src_format;
	int ret;

	if (argc != 5 ) {
        	printf("Usage: test_cdma_p2p src_addr dst_addr data_length desc_num \n");
        	return -1;
    	}

	char* dma_mapped_memory = (char*)mmap_phy_addr(dma_length, dma_offset);
	char* ddr_mapped_memory = (char*)mmap_phy_addr(ddr_length, ddr_offset);
	//char* xlgmac_mapped_memory = (char*)mmap_phy_addr(xlgmac_length, xlgmac_offset);
	printf("mmap success\n");

	uintptr_t src_mem_start_addr = (uintptr_t)strtol(argv[1],&endptr,16);
	uintptr_t dst_mem_start_addr = (u64)strtol(argv[2],&endptr,16);
	u32 data_length = (u32)atoi(argv[3]);
	u8 desc_num = (u32)atoi(argv[4]);
	u32 frame_length = data_length * desc_num;
	printf("src_mem_start_addr:0x%lx dst_mem_start_addr:0x%lx data_length %d frame_length %d\n",
		src_mem_start_addr, dst_mem_start_addr, data_length, frame_length);
	
	uintptr_t src_ddr_addr_virtual = (uintptr_t)(ddr_mapped_memory + src_mem_start_addr);
	uintptr_t dst_ddr_value_virtual = (uintptr_t)(ddr_mapped_memory + dst_mem_start_addr);

	Constructe_Ethernet_Header(src_ddr_addr_virtual);

	write_frame(src_ddr_addr_virtual + eth_header_len, 
		    dst_ddr_value_virtual + eth_header_len, 
		    frame_length - eth_header_len );

	testcase_p2p_tcp_send_mode((char*)dma_mapped_memory,
			     	   (char*)ddr_mapped_memory,
			     	   (u64)src_mem_start_addr,
			     	   data_length,
			    	   frame_length);

	//rcv_poll((char*)dma_mapped_memory);

	testcase_p2p_tcp_rcv_mode((char*)dma_mapped_memory,
			      	  (char*)ddr_mapped_memory,
				  (u64)dst_mem_start_addr,
				   data_length,
				   desc_num);


	compare_frame(src_ddr_addr_virtual + eth_header_len, 
		      dst_ddr_value_virtual + eth_header_len, 
	 	      dst_mem_start_addr + eth_header_len, 
		      frame_length - eth_header_len);

	munmap_dma_reg(dma_length, dma_mapped_memory);
	munmap_dma_reg(ddr_length, ddr_mapped_memory);
	//munmap_dma_reg(xlgmac_length, xlgmac_mapped_memory);

	return 0;
}