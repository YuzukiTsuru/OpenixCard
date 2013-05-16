#ifndef AW_SYSPARA_H
#define AW_SYSPARA_H

#include <inttypes.h>

typedef struct _AW_SYSPARA {
	char magic[8];		/* "SYS_PARA" */
	uint32_t val_0100;	/* 0x00000100 firmwareid? */
	uint32_t val_0;		/* 0x00000000 */
	uint32_t val_1;		/* 0x00000001 */
	uint32_t chip;		/* 0x02000000 */
	uint32_t pid;		/* 0x02000000 */
	uint32_t vid;		/* 0x02000100 */
	uint32_t bid;		/* 0x00000080 */
	uint32_t val_0_2;	/* 0x00000000 */
	uint32_t val_0_3;	/* 0x00000000 */
	uint32_t unknown;	/* 0x00380AC1/3672769 */
	uint32_t pad[16];

	/* offset 0x70 */
	uint32_t dram_baseaddr;
	uint32_t dram_clk;
	uint32_t dram_type;
	uint32_t dram_rank_num;
	uint32_t dram_chip_density;
	uint32_t dram_io_width;
	uint32_t dram_bus_width;
	uint32_t dram_cas;
	uint32_t dram_zq;
	uint32_t dram_odt_en;
	uint32_t dram_size;
	uint32_t dram_tpr0;
	uint32_t dram_tpr1;
	uint32_t dram_tpr2;
	uint32_t dram_tpr3;
	uint32_t dram_tpr4;
	uint32_t dram_tpr5;
	uint32_t dram_emr1;
	uint32_t dram_emr2;
	uint32_t dram_emr3;

	uint32_t pad2[260];

	/* offset 0x4D0 */
	uint32_t mbr_size;
	uint32_t num_partitions;
	struct _AW_PART {
		uint32_t size_hi;
		uint32_t size_lo;
		char class_name[32];
		char name[32];
		uint32_t user_type;
		uint8_t unknown[28];
	} partitions[14];

	uint32_t num_downloads;
	struct _AW_DOWNLOAD {
		char part_name[32];
		char pkt_name[32];
		char verify_file[32];
		uint8_t unknown;
	} downloads[14];

	uint8_t pad3[1436];
} AW_SYSPARA;

#endif /* AW_SYSPARA_H */
