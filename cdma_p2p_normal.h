#ifndef  __CDMA_P2P_NORMAL_H__
#define  __CDMA_P2P_NORMAL_H__

typedef enum {
	CDMA_SEND = 0,
	CDMA_READ = 1,
	CDMA_WRITE = 2,
	CDMA_GENERAL = 3,
	CDMA_RECEIVE = 4,
	CDMA_LOSSY_COMPRESS = 5,
	CDMA_LOSSY_DECOMPRESS = 6,
	CDMA_SYS = 7,
	CDMA_TCP_SEND = 8,
	CDMA_TCP_RECEIVE = 9,
	CDMA_TYPE_NUM,
} CDMA_TYPE;

typedef enum {
	DATA_INT8 = 0,
	DATA_FP16 = 1,
	DATA_FP32 = 2,
	DATA_INT16 = 3,
	DATA_INT32 = 4,
	DATA_BF16 = 5,
	DATA_FP20 = 6,
	DATA_FORMAT_NUM,s
} CDMA_DATA_FORMAT;

typedef struct {
    uint32_t intr_en : 1;
    uint32_t reserved1 : 2;
    uint32_t breakpoint : 1;
    uint32_t cmd_type : 4;
    uint32_t cmd_special_function : 3;
    uint32_t fill_constant_en : 1;
    uint32_t src_data_format : 4;
    uint32_t src_start_addr_h8 : 13;
    uint32_t reserved2 : 3;
    uint32_t dst_start_addr_h8 : 13;
    uint32_t reserved3 : 19;
    uint32_t cmd_length : 32;
    uint32_t src_start_addr_l32 : 32;
    uint32_t dst_start_addr_l32 : 32;
    uint32_t reserved4 : 32;
    uint32_t reserved5 : 32;
    uint32_t reserved6 : 32;
} dma_general_desc;

#define CDMA_PHY_ADDRESS_START 		0x6C08790000LL
#define CDMA_PHY_ADDRESS_END  		0x6C0879FFFFLL
#define CDMA_MEMORY_RANGE 		(CDMA_PHY_ADDRESS_END -\
					CDMA_PHY_ADDRESS_START + 1)

//fffe0000-ffffffff

// #define CDMA_PHY_ADDRESS_START 		0x3ee3ff000LL
// #define CDMA_PHY_ADDRESS_END  		0x3f71fffffLL
// #define CDMA_MEMORY_RANGE 		(CDMA_PHY_ADDRESS_END -\
// 					CDMA_PHY_ADDRESS_START + 1)

#define DDR_PHY_ADDRESS_START 		0x0000000000LL
#define DDR_PHY_ADDRESS_END 		0x007FFFFFFFLL
#define DDR_MEMORY_RANGE 		(DDR_PHY_ADDRESS_END -\
					DDR_PHY_ADDRESS_START + 1)

// #define DDR_PHY_ADDRESS_START 		0x3ed400000LL
// #define DDR_PHY_ADDRESS_END 		0x3ee1fffffLL
// #define DDR_MEMORY_RANGE 		(DDR_PHY_ADDRESS_END -\
// 					DDR_PHY_ADDRESS_START + 1)

#define DDR_PHY_OFFSET			0x1000

// #define CMD_REG_BASE			0x6C08790000
// #define DESC_UPDT			0x6C08790400
// #define CSR_REG_BASE			0x6C08791000

#define CMD_REG_OFFSET			0x0000
#define DESC_UPDT_OFFSET		0x0400
#define CSR_REG_OFFSET			0x1000

// CMD_GENERAL_REG
#define CDMA_CMD_INTR_ENABLE		0
#define CDMA_CMD_BREAKPOINT		3
#define CDMA_CMD_CMD_TYPE		4
#define CDMA_CMD_SPECIAL_FUNCTION	8
#define CDMA_CMD_FILL_CONSTANT_EN	11
#define CDMA_CMD_SRC_DATA_FORMAT	12
#define CDMA_CMD_SRC_START_ADDR_H13	16
#define CDMA_CMD_DST_START_ADDR_H13	32

#define CDMA_CMD_CMD_LENGTH		0
#define CDMA_CMD_SRC_START_ADDR_L32	32

#define CDMA_CMD_DST_START_ADDR_L32	0

// DESCRIPTOR_UPDT
#define REG_DESCRIPTOR_UPDATE 0

// CSR_REG_BASE
#define	CDMA_CSR_0_OFFSET		0x0
#define	CDMA_CSR_1_OFFSET		0x4	//reg_rcv_addr_h32
#define CDMA_CSR_4_OFFSET		0x10
#define CDMA_CSR_5_OFFSET		0x14
#define CDMA_CSR_6_OFFSET		0x18
#define CDMA_CSR_9_OFFSET		0x2c
#define CDMA_CSR_20_Setting		0x58	//intr
#define REG_BASE_ADDR_REGINE0		0x5c
#define CDMA_CSR_141_OFFSET		0x23c
#define CDMA_CSR_142_OFFSET		0x240
#define MTL_FAB_CSR_00			0x3c0

// CDMA_CSR_0
#define	REG_DES_MODE_ENABLE 	0
#define	REG_SYNC_ID_RESET 	1
#define REG_DES_CLR		2
#define REG_CONNECTION_ON	6
#define	REG_HW_REPLAY_EN	7



// CDMA_CSR_4
#define REG_REPLAY_MAX_TIME 		19	 //bit 19~22
#define REG_SEC_LENGTH			16	//bit 16~18
#define REG_DES_MAX_ARLEN		23	//bit 23~25
#define REG_REPLAY_RECONTINUE		26

// CDMA_CSR_5
#define REG_TIMEOUT_MAXTIME		0 	//bit 0~19

// CDMA_CSR_6
#define REG_CDMA_VLAN_ID		0	//bit 0~15
#define	REG_CDMA_VLAN_ID_VTIR		16	//bit 16~17
#define REG_CDMA_CPC			18	//bit 18~19
#define REG_CDMA_SAIC			20	//bit 20~21
#define REG_CDMA_CIC			22	//bit 22~23

// CDMA_CSR_9
#define REG_DES_ADDR_L32		0

// CDMA_CSR_10

#define REG_DES_ADDR_H1			0
// CDMA_CSR_20_Setting
#define INTR_CMD_DONE_STATUS		0

// CDMA_CSR_141
#define  REG_INTRA_DIE_READ_ADDR_H8	0	//bit 0~7
#define  REG_INTRA_DIE_WRITE_ADDR_H8	8	//bit 8~15
// CDMA_CSR_142
#define reg_des_read_addr_h8		0	//bit 0~7
#define reg_des_write_addr_h8		8	//bit 8~15

// MTL_FAB_CSR_00
#define REG_CDMA_TX_CH_ID		0	//bit 0~3
#define REG_CDMA_RX_CH_ID		4	//bit 4~7
#define REG_CDMA_FAB_ARBITRATION	8	//bit 8~9
#define REG_CDMA_FAB_BYPASS		10






// static inline void write128(uint64_t _addr, uint64_t _l64, uint64_t _h64) {
//     asm volatile ("sdd %0, %1, (%2), 0, 4"
// 	      :
// 	      : "r"(_l64), "r"(_h64), "r"(_addr));
// }

// #define WRITE_CMD_CDMA(cmd_reg_base, offset, h64, l64) \
// 	write128(cmd_reg_base + (offset << 4), l64, h64)



#endif
