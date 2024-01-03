#ifndef  __XLGMAC_VT_H__
#define  __XLGMAC_VT_H__

#include "bm_types.h"
typedef enum {
	EINVAL = 1,
	ETIME_OUT = 2,

} ERROR_STATUS;

typedef enum {
	MTL_QUEUE_AVB = 0x0,
	MTL_QUEUE_DCB = 0x1,
	
} QUEUE_MODE;


#define FLAG_FILTER_ADD     		1UL << 0
#define FLAG_FILTER_DEL      		1UL << 1


#define XLGMAC_ETH_BASE_START   	0x6c08000000ULL
#define XLGMAC_ETH_BASE_END		0X6C0803FFFFULL
#define XLGMAC_MEMORY_RANGE 		(XLGMAC_ETH_BASE_END -\
					XLGMAC_ETH_BASE_START + 1)

#define TX_CONFIG			0x00000000
#define MAC_Rx_Configuration 		0x00000004
#define PACKET_FILTER			0x00000008
#define VLAN_TAG_CTRL			0x00000050
#define VLAN_TAG_DATA			0x00000054
#define RxQ_Enable_Ctrl0		0x00000140
#define MTL_RxQn_Operation_Mode(x)	(0x00001140 + (0x80 * (x)))		
#define MTL_TxQn_Operation_Mode(x)	(0x00001100 + (0x80 * (x)))
#define RxQ_Priority_Mapping_Ctrl(x)	(0x00000160 + (0x04 * (x)))
#define MTL_RXQ_DMA_MAP(x)		(0x00001030 + (0x04 * (x)))
#define DMA_CH_TX_CONTROL(x)		(0x00003104 + (0x80 * (x)))
#define DMA_CH_RX_CONTROL(x)		(0x00003108 + (0x80 * (x)))

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
#define VLAN_TAG_DATA_DOVLTC		18
#define VLAN_TAG_CTRL_DMACHEN		24
#define VLAN_TAG_CTRL_DMACHN		25

/* MTL_RxQn_Operation_Mode*/
#define RTC				0
#define RQS				16

/* MTL_TxQn_Operation_Mode */
#define FTQ				0
#define TSF				1
#define TXQEN				2//2~3
#define TTC				4//4~6
#define	Q2TCMAP				8//8~10
#define TQS				16//16~??

/* RxQ_Priority_Mapping_Ctrl */
#define	PSRQ_SHIFT(x)			((x) * 8)

/* MTL_RXQ_DMA_MAP0 */
#define DDMACH(x)			(((x + 1) * 8) - 1)

/* XGMAC_DMA_CH_TX_CONTROL */
#define TXST 				0

/* XGMAC_DMA_CH_RX_CONTROL */
#define	RXST				0

/* MAC_Rx_Configuration */
#define RE				0
#define	ACS				1
#define CST				2
#define LM				10






















#endif