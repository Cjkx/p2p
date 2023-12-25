#include "cdma_p2p_common.h"
#include "cdma_pmu.h"
#include "cdma_p2p_normal.h"

void enable_cdma_perf_monitor(char* dma_base_mmap, u64 start_addr, u32 size)
{
	u32 start_addr_l32 = (u32)(start_addr >> 7);
	u32 start_addr_h1 = (u32)(start_addr >> 39);
	u64 end_addr = start_addr + size;
	u32 end_addr_l32 = (u32)(end_addr >> 7);
	u32 end_addr_h1 = (u32)(end_addr >> 39);
	u32 tmp;

	printf("enable cdma perf monitor size = 0x%x, start addr = 0x%lx, end addr = 0x%lx\n", 
		 size, start_addr, end_addr);
	
	memset((u32 *)start_addr, 0, size);
	//flush_dcache_range(start_addr, size);

	uintptr_t csr_reg_base = (uintptr_t)(dma_base_mmap + CSR_REG_OFFSET);

	sg_write32(csr_reg_base, CDMA_CSR_PMU_START_ADDR_L32, start_addr_l32);
	sg_write32(csr_reg_base, CDMA_CSR_PMU_START_ADDR_H1, start_addr_h1);
	sg_write32(csr_reg_base, CDMA_CSR_PMU_END_ADDR_L32, end_addr_l32);
	sg_write32(csr_reg_base, CDMA_CSR_PMU_END_ADDR_H1, end_addr_h1);
	debug_log("write CDMA_CSR_PMU_START_ADDR_L32 to 0x%x\n", sg_read32(csr_reg_base, CDMA_CSR_PMU_START_ADDR_L32));
        debug_log("write CDMA_CSR_PMU_START_ADDR_H1 to 0x%x\n", sg_read32(csr_reg_base, CDMA_CSR_PMU_START_ADDR_H1));
	debug_log("write CDMA_CSR_PMU_END_ADDR_L32 to 0x%x\n", sg_read32(csr_reg_base, CDMA_CSR_PMU_END_ADDR_L32));
        debug_log("write CDMA_CSR_PMU_END_ADDR_H1 to 0x%x\n", sg_read32(csr_reg_base, CDMA_CSR_PMU_END_ADDR_H1));

	tmp = sg_read32(csr_reg_base, CDMA_CSR_142_OFFSET) |
		(0x80 << REG_DES_WRITE_ADDR_H8);
	sg_write32(csr_reg_base, CDMA_CSR_142_OFFSET, tmp);
	
	// enable pmu
	tmp = sg_read32(csr_reg_base, CDMA_CSR_0_OFFSET) |
		(1 << REG_PERF_MONITOR_EN);
	sg_write32(csr_reg_base, CDMA_CSR_0_OFFSET, tmp);
	debug_log("write CDMA_CSR_PERF_MONITOR_EN to 0x1\n");
}

void disable_cdma_perf_monitor(char* dma_base_mmap)
{
	u32 tmp;

	uintptr_t csr_reg_base = (uintptr_t)(dma_base_mmap + CSR_REG_OFFSET);

	tmp = sg_read32(csr_reg_base, CDMA_CSR_0_OFFSET) &
		~(1 << REG_PERF_MONITOR_EN);
	sg_write32(csr_reg_base, CDMA_CSR_0_OFFSET, tmp);
	debug_log("write CDMA_CSR_PERF_MONITOR_EN to 0x0\n");
}

void show_cdma_pmu_data(u64 start_addr, int index)
{
	cdma_pmu_item_t *p = (cdma_pmu_item_t *)(start_addr) + index;

	//inval_dcache_range(start_addr + index * sizeof(cdma_pmu_item_t), sizeof(cdma_pmu_item_t));

	printf("start addr is 0x%lx\n", mmio_read_64(start_addr));

	printf("cdma record #%d, inst_id=0x%x inst_start_time=0x%x, inst_end_time=0x%x\n", index, p->inst_id, p->inst_start_time, p->inst_end_time);
	printf("m0_data_aw_cntr=%d, m0_data_w_cntr=%d, m0_data_ar_cntr=%d\n", p->m0_data_aw_cntr, p->m0_data_w_cntr, p->m0_data_ar_cntr);
	printf("m0_data_wr_valid_cntr=%d, m0_data_wr_stall_cntr=%d, m0_data_rd_valid_cntr=%d, m0_data_rd_stall_cntr=%d\n", p->m0_data_wr_valid_cntr, p->m0_data_wr_stall_cntr, p->m0_data_rd_valid_cntr, p->m0_data_rd_stall_cntr);
	printf("ati_data_valid_cntr=%d, ati_data_stall_cntr=%d, ari_data_valid_cntr=%d, ari_data_stall_cntr=%d\n", p->ati_data_valid_cntr, p->ati_data_stall_cntr, p->ari_data_valid_cntr, p->ari_data_stall_cntr);
	printf("ati_txfifo_stall_cntr=%d, replay_number=%d, m0_data_b_st=%d, m0_data_b_end=%d\n", p->ati_txfifo_stall_cntr, p->replay_number, p->m0_data_b_st, p->m0_data_b_end);
	printf("m0_data_ar_st=%d, m0_data_ar_end=%d, m0_data_aw_st=%d, m0_data_aw_end=%d\n", p->m0_data_ar_st, p->m0_data_ar_end, p->m0_data_aw_st, p->m0_data_aw_end);
	printf("m0_data_rd_st=%d, m0_data_rd_end=%d, m0_data_wr_st=%d, m0_data_wr_end=%d\n", p->m0_data_rd_st, p->m0_data_rd_end, p->m0_data_wr_st, p->m0_data_wr_end);
	printf("ati_data_st=%d, ati_data_end=%d, ari_data_st=%d, ari_data_end=%d\n", p->ati_data_st, p->ati_data_end, p->ari_data_st, p->ari_data_end);
}
