#ifndef CDMA_PMU_H_
#define CDMA_PMU_H_

#define USING_PMU

typedef struct {
	// H0
	unsigned int inst_start_time;
	unsigned int inst_end_time;
	unsigned int inst_id; // bit 0-23 inst_id; bit 24 thread id
	unsigned int reserved0;
	// H1
	unsigned int m0_data_aw_cntr;
	unsigned int m0_data_w_cntr;
	unsigned int m0_data_ar_cntr;
	unsigned int reserved1;
	// H2
	unsigned int m0_data_wr_valid_cntr;
	unsigned int m0_data_wr_stall_cntr;
	unsigned int m0_data_rd_valid_cntr;
	unsigned int m0_data_rd_stall_cntr;
	// H3
	unsigned int ati_data_valid_cntr;
	unsigned int ati_data_stall_cntr;
	unsigned int ari_data_valid_cntr;
	unsigned int ari_data_stall_cntr;
	// H4
	unsigned int ati_txfifo_stall_cntr;
	unsigned int replay_number;
	unsigned int m0_data_b_st;
	unsigned int m0_data_b_end;
	// H5
	unsigned int m0_data_ar_st;
	unsigned int m0_data_ar_end;
	unsigned int m0_data_aw_st;
	unsigned int m0_data_aw_end;
	// H6
	unsigned int m0_data_rd_st;
	unsigned int m0_data_rd_end;
	unsigned int m0_data_wr_st;
	unsigned int m0_data_wr_end;
	// H7
	unsigned int ati_data_st;
	unsigned int ati_data_end;
	unsigned int ari_data_st;
	unsigned int ari_data_end;
} cdma_pmu_item_t;


void enable_cdma_perf_monitor(char* dma_base_mmap, u64 start_addr, u32 size);
void disable_cdma_perf_monitor(char* dma_base_mmap);
void show_cdma_pmu_data(u64 start_addr, int index);

#endif