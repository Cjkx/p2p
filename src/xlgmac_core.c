#include "xlgmac_core.h"
#include "cdma_p2p_common.h"



#define VLAN_FILTER_NUM		4

u32 vlan_filter[VLAN_FILTER_NUM];

static int xlgmac_rx_queue_prio(char* xlgmac_mapped_memory, u8 queue, u8 prio){
	u32 val;
	int ret = queue;
	int x = queue;
	int reg_index = queue / 4;
	uintptr_t xlgmac_reg_addr = (uintptr_t)(xlgmac_mapped_memory);
	prio = prio + 1;
	switch (x) {
	case 0 ... 3:
		val = sg_read32(xlgmac_reg_addr, RxQ_Priority_Mapping_Ctrl(reg_index));
		val &= ~(0xff << PSRQ_SHIFT(x));
		val |= (prio << PSRQ_SHIFT(x));
		sg_write32(xlgmac_reg_addr, RxQ_Priority_Mapping_Ctrl(reg_index), val);
		break;
	case 4 ... 7:
		val = sg_read32(xlgmac_reg_addr, RxQ_Priority_Mapping_Ctrl(reg_index));
		val &= ~(0xff << PSRQ_SHIFT(x - 4));
		val |= (prio << PSRQ_SHIFT(x - 4));
		sg_write32(xlgmac_reg_addr, RxQ_Priority_Mapping_Ctrl(reg_index), val);
		break;
	case 8 ... 11:
		val = sg_read32(xlgmac_reg_addr, RxQ_Priority_Mapping_Ctrl(reg_index));
		val &= ~(0xff << PSRQ_SHIFT(x - 8));
		val |= (prio << PSRQ_SHIFT(x - 8));
		sg_write32(xlgmac_reg_addr, RxQ_Priority_Mapping_Ctrl(reg_index), val);
		break;
	case 12 ... 15:
		val = sg_read32(xlgmac_reg_addr, RxQ_Priority_Mapping_Ctrl(reg_index));
		val &= ~(0xff << PSRQ_SHIFT(x - 12));
		val |= (prio << PSRQ_SHIFT(x - 12));
		sg_write32(xlgmac_reg_addr, RxQ_Priority_Mapping_Ctrl(reg_index), val);
		break;
	default:
		printf("The index of CDMA channels exceeds the maximum value \n");
		ret = -EINVAL;
		break;
	}

	printf("queue %d write val 0x%x to MTL_RXQ_DMA_MAP addr:0x%x \n", 
		queue, val, RxQ_Priority_Mapping_Ctrl(reg_index));
	printf("queue %d sets the priority to %d\n", queue, prio - 1);

	return ret;
}

static int xlgmac_dmap_mtl_to_dma(char* xlgmac_mapped_memory, u8 channel){
	u32 val;
	int ret = channel;
	int x = channel;
	int reg_index = channel / 4;
	uintptr_t xlgmac_reg_addr = (uintptr_t)(xlgmac_mapped_memory);

	switch (x) {
	case 0 ... 3:
		val = sg_read32(xlgmac_reg_addr, MTL_RXQ_DMA_MAP(reg_index));
		val |= (1 << DDMACH(x));
		sg_write32(xlgmac_reg_addr, MTL_RXQ_DMA_MAP(reg_index), val);
		break;
	case 4 ... 7:
		val = sg_read32(xlgmac_reg_addr, MTL_RXQ_DMA_MAP(reg_index));
		val |= (1 << DDMACH(x - 4));
		sg_write32(xlgmac_reg_addr, MTL_RXQ_DMA_MAP(reg_index), val);
		break;
	case 8 ... 11:
		val = sg_read32(xlgmac_reg_addr, MTL_RXQ_DMA_MAP(reg_index));
		val |= (1 << DDMACH(x - 8));
		sg_write32(xlgmac_reg_addr, MTL_RXQ_DMA_MAP(reg_index), val);
		break;
	case 12 ... 15:
		val = sg_read32(xlgmac_reg_addr, MTL_RXQ_DMA_MAP(reg_index));
		val |= (1 << DDMACH(x - 12));
		sg_write32(xlgmac_reg_addr, MTL_RXQ_DMA_MAP(reg_index), val);
		break;
	default:
		printf("The index of CDMA channels exceeds the maximum value \n");
		break;
	}
	printf("Channel %d write val 0x%x to MTL_RXQ_DMA_MAP addr:0x%x \n", 
		channel, val, MTL_RXQ_DMA_MAP(reg_index));
	printf("Channel %d enable Dynamic DMA channel mapping\n",channel);

	return ret;
}

static int xlgmac_write_vlan_filter(char* xlgmac_mapped_memory, u8 index, u16 vlan_id){
	u32 val = 0;
	u32 read_val;
	int i, timeout = 100;

	if (index >= VLAN_FILTER_NUM)
		return -EINVAL;

	uintptr_t xlgmac_reg_addr = (uintptr_t)(xlgmac_mapped_memory);
	val = 0;
	/* config vlan_tag_data reg */
	val =  (CDMA_RX_CHANNEL << VLAN_TAG_CTRL_DMACHN) | ( 1 << VLAN_TAG_CTRL_DMACHEN) |
		(1 << VLAN_TAG_DATA_DOVLTC) | (0 << VLAN_TAG_DATA_ETV) | (1 <<  VLAN_TAG_DATA_VEN) | vlan_id;
	sg_write32(xlgmac_reg_addr, VLAN_TAG_DATA, val);
	printf("[VLAN_TAG_DATA] offset:0x%x, write value:0x%x\n", VLAN_TAG_DATA, val);

	/* config vlan_tag_control reg */
	val = sg_read32(xlgmac_reg_addr, VLAN_TAG_CTRL);
	val &= ~((0x1f << VLAN_TAG_CTRL_OFS) |
		(1 << VLAN_TAG_CTRL_CT) |
		(1 << VLAN_TAG_CTRL_OB));
	val |= (index << VLAN_TAG_CTRL_OFS) | (1 << VLAN_TAG_CTRL_OB);
	sg_write32(xlgmac_reg_addr, VLAN_TAG_CTRL, val);
	printf("[VLAN_TAG_CTRL] expected value:0x%x\n", val);

	/* wait till the OB bit is reset to do the next write */
	for (i = 0; i < timeout; i++) {
		val = sg_read32(xlgmac_reg_addr, VLAN_TAG_CTRL);
		if (!(val & (1  << VLAN_TAG_CTRL_OB)))
			return 0;
		usleep(1);
	}

	printf("Timeout accessing MAC_VLAN_Tag_Filter\n");

	return -ETIME_OUT;

}

int xlgmac_rx_queue_enable(char* xlgmac_mapped_memory, u8 mode, u32 queue){
	u32 val;

	uintptr_t xlgmac_reg_addr = (uintptr_t)(xlgmac_mapped_memory);

	val = sg_read32(xlgmac_reg_addr, RxQ_Enable_Ctrl0);
	val &= ~(0x3 << queue * 2);
	if (mode == MTL_QUEUE_AVB){
		val |= (0x1 << queue * 2);
		printf("rx queue_%d enable AVB mode\n", queue);
	}
	else if (mode == MTL_QUEUE_DCB){
		val |= (0x2 << queue * 2);
		printf("rx queue_%d enable DCB mode\n", queue);
	}
		
	sg_write32(xlgmac_reg_addr, RxQ_Enable_Ctrl0, val);
	printf("[MAC_RxQ_Enable_Ctrl0] write value:0x%x\n", val);
}

int xlgmac_tx_queue_enable(char* xlgmac_mapped_memory, u8 mode, u32 queue){
	u32 val;

	uintptr_t xlgmac_reg_addr = (uintptr_t)(xlgmac_mapped_memory);
	val = sg_read32(xlgmac_reg_addr, MTL_TxQn_Operation_Mode(queue));
	val &= ~(0x3 << TXQEN);
	if (mode == MTL_QUEUE_AVB){
		val |= (0x1 << TXQEN);
		printf("tx queue_%d enable AVB mode\n", queue);
	}
		
	else if (mode == MTL_QUEUE_DCB){
		val |= (0x2 << TXQEN);
		printf("rx queue_%d enable DCB mode\n", queue);
	}

	sg_write32(xlgmac_reg_addr, MTL_TxQn_Operation_Mode(queue), val);
	printf("[MTL_TxQ%d_Operation_Mode] addr:0x%x write value:0x%x\n", 
		MTL_TxQn_Operation_Mode(queue),queue, val);

}

static void xlgmac_dma_channel_disable(char* xlgmac_mapped_memory, u8 channel){
	u32 val;

	uintptr_t xlgmac_reg_addr = (uintptr_t)(xlgmac_mapped_memory);

	val = sg_read32(xlgmac_reg_addr,DMA_CH_TX_CONTROL(channel));
	val &= ~(1 << TXST);
	sg_write32(xlgmac_reg_addr, DMA_CH_TX_CONTROL(channel), val);

	printf("channel %d TX DMA disable\n", channel);

	val = sg_read32(xlgmac_reg_addr,DMA_CH_RX_CONTROL(channel));
	val &= ~(1 << RXST);
	sg_write32(xlgmac_reg_addr, DMA_CH_RX_CONTROL(channel), val);

	printf("channel %d RX DMA disable\n", channel);
}



int xlgmac_add_vlan_filter(char* xlgmac_mapped_memory, u8 index, u16 vlan_id) {

	int ret = 0;

	// for(int index = 0; index < VLAN_FILTER_NUM; index++)
	// 	xlgmac_write_vlan_filter(xlgmac_mapped_memory,index,index + 10);
	vlan_filter[index] = vlan_id;
	printf("vlan_filter[index] :%d vlan id %d\n", vlan_filter[index],vlan_id );
	ret = xlgmac_write_vlan_filter(xlgmac_mapped_memory, index, vlan_id);

	return ret;
}

int xlgmac_del_vlan_filter(char* xlgmac_mapped_memory, u8 index, u16 vlan_id) {

	int ret;
	printf("vlan_filter[index] :%d vlan id %d\n", vlan_filter[index],vlan_id );
	if(vlan_id == vlan_filter[index]) {
		ret = xlgmac_write_vlan_filter(xlgmac_mapped_memory, index, 0);
		printf("del filter_%d vid %u \n", index, vlan_id);
	} else {
		printf("no filter's vid = %u \n", vlan_id);
	}
	
	return ret;
}



int main(int argc, char** argv){

	size_t xlgmac_length = XLGMAC_MEMORY_RANGE;
	off_t  xlgmac_offset = XLGMAC_ETH_BASE_START;
	u8 channel = CDMA_RX_CHANNEL;
	u8 filter_index = 0;
	u32 option;
	int ch;
	int ret;
	char* xlgmac_mapped_memory = (char*)mmap_phy_addr(xlgmac_length, xlgmac_offset);
	printf("mmap successful\n");

	if(argc < 3){
		printf("invalid argc! test_xlgmac_core a/d vid");
		goto munmap;
	}

	u32 vid = (u32)atoi(argv[2]);
	u32 prio = (u32)atoi(argv[3]);
	if (!strcmp(argv[1], "a") && argc == 4) {
		ret = xlgmac_add_vlan_filter(xlgmac_mapped_memory, filter_index, vid);
		if(ret == 0){
			printf("add vlan filter_%d vid:%u successful!\n",filter_index, vid);
		} else {
			printf("add vlan filter_%d vid:%u fail!,ret = %d\n",filter_index, vid, ret);
		}
		xlgmac_rx_queue_prio(xlgmac_mapped_memory, CDMA_RX_CHANNEL, prio);
		xlgmac_dmap_mtl_to_dma(xlgmac_mapped_memory, CDMA_RX_CHANNEL);
		
	} else if (!strcmp(argv[1], "d") && argc == 4) {
		ret = xlgmac_del_vlan_filter(xlgmac_mapped_memory, filter_index, vid);
		if (ret == 0) {
			printf("del vlan filter_%d vid:%u successful!\n",filter_index, vid);
		} else {
			printf("del vlan filter_%d vid:%u fail!,ret = %d\n",filter_index, vid, ret);
		}
	} else {
		printf("invalid argc! test_xlgmac_core a/d vid");
	}

	/* Set RX priorities */
	//xlgmac_rx_queue_prio(xlgmac_mapped_memory, channel, prio);
	
	/* Dynastic Map RX MTL to DMA channels*/
	//xlgmac_dmap_mtl_to_dma(xlgmac_mapped_memory, channel);

munmap:
	munmap_dma_reg(xlgmac_length, xlgmac_mapped_memory);
}



