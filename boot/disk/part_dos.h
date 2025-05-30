/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef _DISK_PART_DOS_H
#define _DISK_PART_DOS_H

#define DOS_PART_DISKSIG_OFFSET	0x1b8
#define DOS_PART_TBL_OFFSET	0x1be
#define DOS_PART_MAGIC_OFFSET	0x1fe
#define DOS_PBR_FSTYPE_OFFSET	0x36
#define DOS_PBR32_FSTYPE_OFFSET	0x52
#define DOS_PBR_MEDIA_TYPE_OFFSET	0x15
#define DOS_MBR	0
#define DOS_PBR	1
#define DOS_PART_TYPE_EXTENDED		0x05
#define DOS_PART_TYPE_EXTENDED_LBA	0x0F
#define DOS_PART_TYPE_EXTENDED_LINUX	0x85

#define DOS_PART_DEFAULT_GAP		2048

typedef struct dos_partition {
	unsigned char boot_ind;		/* 0x80 - active			*/
	unsigned char head;		/* starting head			*/
	unsigned char sector;		/* starting sector			*/
	unsigned char cyl;		/* starting cylinder			*/
	unsigned char sys_ind;		/* What partition type			*/
	unsigned char end_head;		/* end head				*/
	unsigned char end_sector;	/* end sector				*/
	unsigned char end_cyl;		/* end cylinder				*/
	unsigned char start4[4];	/* starting sector counting from 0	*/
	unsigned char size4[4];		/* nr of sectors in partition		*/
} dos_partition_t;

#if defined(CONFIG_ARCH_SSTAR) && CONFIG_IS_ENABLED(CMD_SSTAR_MMC_FDISK)

#define BLOCK_SIZE			512
#define BLOCK_END			0xFFFFFFFF
#define _10MB				(10*1024*1024)
#define _100MB				(100*1024*1024)
#define _8_4GB				(1023*254*63)
#define _MB_				(1024*1024)

#define CHS_MODE			0
#define LBA_MODE			!(CHS_MODE)

#define CFG_PARTITION_START 		0x800
#define CFG_PARTITION_RESERVE		0x800
#define CFG_EXT_PARTITION_RESERVE	0x10

typedef struct
{
	unsigned int		C_start;
	unsigned int		H_start;
	unsigned int		S_start;

	unsigned int		C_end;
	unsigned int		H_end;
	unsigned int		S_end;

	unsigned int		available_block;
	unsigned int		unit;
	unsigned int		total_block_count;
	unsigned int		addr_mode;	// LBA_MODE or CHS_MODE
} SDInfo;

typedef struct
{
	unsigned char		bootable;
	unsigned char		partitionId;

	unsigned int		C_start;
	unsigned int		H_start;
	unsigned int		S_start;

	unsigned int		C_end;
	unsigned int		H_end;
	unsigned int		S_end;

	unsigned int		block_start;
	unsigned int		block_count;
	unsigned int		block_end;
} PartitionInfo;
#endif

#endif	/* _DISK_PART_DOS_H */
