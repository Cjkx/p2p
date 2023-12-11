#ifndef  __XLGMAC_VT_H__
#define  __XLGMAC_VT_H__

#include "bm_types.h"
typedef enum {
	EINVAL = 1,
	ETIME_OUT = 2,

} ERROR_STATUS;

#define XLGMAC_ETH_BASE_START   	0x6c08000000ULL
#define XLGMAC_ETH_BASE_END		0X6C0803FFFFULL
#define XLGMAC_MEMORY_RANGE 		(XLGMAC_ETH_BASE_END -\
					XLGMAC_ETH_BASE_START + 1)


#define PACKET_FILTER			0x00000008
#define VLAN_TAG_CTRL			0x00000050
#define VLAN_TAG_DATA			0x00000054


/* PACKET_FILTER */
#define PACKET_FILTER_VTFE		16

/* VLAN Tag Control */
#define VLAN_TAG_CTRL_OB		0
#define VLAN_TAG_CTRL_CT		1
#define VLAN_TAG_CTRL_OFS		2 //bit2~6
#define VLAN_TAG_CTRL_ETV		16
#define VLAN_TAG_CTRL_ESVL		18
#define VLAN_TAG_CTRL_EVLRXS		24

/* VLAN_TAG_DATA */
#define VLAN_TAG_DATA_VEN		16
#define VLAN_TAG_DATA_ETV		17
#define VLAN_TAG_CTRL_DMACHEN		24
#define VLAN_TAG_CTRL_DMACHN		25


int xlgmac_write_vlan_filter(char* xlgmac_mapped_memory, u8 index, u16 vlan_id);




















#endif