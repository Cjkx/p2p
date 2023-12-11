#include "xlgmac_vt.h"
#include "cdma_p2p_common.h"
#include "bm_types.h"


#define VLAN_FILTER_NUM		4

// void* mmap_xlgmac_phy_addr(void) {
// 	int mem_fd;
//     	void *mapped_memory;
// 	mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
// 	if (mem_fd == -1) {
// 		printf("Error opening /dev/mem");
// 		exit(EXIT_FAILURE);
//    	}
// 	mapped_memory = mmap(NULL, XLGMAC_MEMORY_RANGE, PROT_READ | PROT_WRITE, MAP_SHARED, 
// 				mem_fd, XLGMAC_ETH_BASE_START);
// 	if (mapped_memory == MAP_FAILED) {
// 		printf("Error mapping memory");
// 		close(mem_fd);
// 		exit(EXIT_FAILURE);
//     	}
// 	close(mem_fd);

//     	return mapped_memory;
// }

// static int xlgmac_vlan_config(char* xlgmac_mapped_memory,u32 vid){
// 	u32 value;
// 	uintptr_t xlgmac_reg_addr = (uintptr_t)(xlgmac_mapped_memory);

// 	/* When this bit is set, the MAC drops the VLAN tagged 
// 	 *  packets that do not match the VLAN Tag 
// 	 */
// 	value = sg_read32(xlgmac_reg_addr,PACKET_FILTER_VTFE);
// 	value |= (1 << PACKET_FILTER_VTFE);
// 	sg_write32(xlgmac_reg_addr,PACKET_FILTER_VTFE, value);

// 	value = sg_read32(xlgmac_reg_addr, VLAN_TAG_CTRL);
// 	value &= ~(1 << VLAN_TAG_CTRL_VTHM);



// }

int xlgmac_write_vlan_filter(char* xlgmac_mapped_memory, u8 index, u16 vlan_id){
	u32 val = 0;
	u32 read_val;
	int i, timeout = 100;

	if (index >= VLAN_FILTER_NUM)
		return -EINVAL;

	uintptr_t xlgmac_reg_addr = (uintptr_t)(xlgmac_mapped_memory);

	/* config vlan_tag_data reg */
	val |=  (CDMA_RX_CHANNEL << VLAN_TAG_CTRL_DMACHN) | ( 1 << VLAN_TAG_CTRL_DMACHEN) |
		(1 << VLAN_TAG_DATA_ETV) | (1 <<  VLAN_TAG_DATA_VEN) | vlan_id;
	sg_write32(xlgmac_reg_addr, VLAN_TAG_DATA, val);
	printf("[VLAN_TAG_DATA] expected value:0x%x\n", val);

	/* config vlan_tag_control reg */
	val = sg_read32(xlgmac_reg_addr, VLAN_TAG_CTRL);
	val &= ~((0x1f << VLAN_TAG_CTRL_OFS) |
		VLAN_TAG_CTRL_CT |
		VLAN_TAG_CTRL_OB);
	val |= (index << VLAN_TAG_CTRL_OFS) | (1 << VLAN_TAG_CTRL_OB);
	sg_write32(xlgmac_reg_addr, VLAN_TAG_CTRL, val);
	printf("[VLAN_TAG_CTRL] expected value:0x%x\n", val);
	read_val = sg_read32(xlgmac_reg_addr, VLAN_TAG_CTRL);
	printf("[VLAN_TAG_CTRL] real value:0x%x\n", read_val);

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

int xlgmac_add_vlan_filter(void) {
	size_t xlgmac_length = XLGMAC_MEMORY_RANGE;
	off_t  xlgmac_offset = XLGMAC_ETH_BASE_START;

	char* xlgmac_mapped_memory = (char*)mmap_phy_addr(xlgmac_length, xlgmac_offset);

	// for(int index = 0; index < VLAN_FILTER_NUM; index++)
	// 	xlgmac_write_vlan_filter(xlgmac_mapped_memory,index,index + 10);

	xlgmac_write_vlan_filter(xlgmac_mapped_memory, 0, 10);

	munmap_dma_reg(xlgmac_length, xlgmac_mapped_memory);
}




