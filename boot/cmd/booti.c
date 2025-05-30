// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <bootm.h>
#include <command.h>
#include <image.h>
#include <irq_func.h>
#include <lmb.h>
#include <log.h>
#include <mapmem.h>
#include <asm/global_data.h>
#include <linux/kernel.h>
#include <linux/sizes.h>

DECLARE_GLOBAL_DATA_PTR;
/*
 * Image booting support
 */
static int booti_start(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[], bootm_headers_t *images)
{
	int ret;
	ulong ld;
	ulong relocated_addr;
	ulong image_size;
	uint8_t *temp;
	ulong dest;
	ulong dest_end;
	unsigned long comp_len;
	unsigned long decomp_len;
	int ctype;

	ret = do_bootm_states(cmdtp, flag, argc, argv, BOOTM_STATE_START,
			      images, 1);

	/* Setup Linux kernel Image entry point */
	if (!argc) {
		ld = image_load_addr;
		debug("*  kernel: default image load address = 0x%08lx\n",
				image_load_addr);
	} else {
		ld = hextoul(argv[0], NULL);
		debug("*  kernel: cmdline image address = 0x%08lx\n", ld);
	}

	temp = map_sysmem(ld, 0);
	ctype = image_decomp_type(temp, 2);
	if (ctype > 0) {
		dest = env_get_ulong("kernel_comp_addr_r", 16, 0);
		comp_len = env_get_ulong("kernel_comp_size", 16, 0);
		if (!dest || !comp_len) {
			puts("kernel_comp_addr_r or kernel_comp_size is not provided!\n");
			return -EINVAL;
		}

#ifndef CONFIG_SSTAR_ANDROID_BOOTLOADER
		if (dest < gd->ram_base || dest > gd->ram_top) {
			puts("kernel_comp_addr_r is outside of DRAM range!\n");
			return -EINVAL;
		}
#endif

		debug("kernel image compression type %d size = 0x%08lx address = 0x%08lx\n",
			ctype, comp_len, (ulong)dest);
		decomp_len = comp_len * 10;
		ret = image_decomp(ctype, 0, ld, IH_TYPE_KERNEL,
				 (void *)dest, (void *)ld, comp_len,
				 decomp_len, &dest_end);
		if (ret)
			return ret;
		/* dest_end contains the uncompressed Image size */
		memmove((void *) ld, (void *)dest, dest_end);
	}
	unmap_sysmem((void *)ld);

	ret = booti_setup(ld, &relocated_addr, &image_size, false);
	if (ret != 0)
		return 1;

	/* Handle BOOTM_STATE_LOADOS */
	if (relocated_addr != ld) {
		printf("Moving Image from 0x%lx to 0x%lx, end=%lx\n", ld,
		       relocated_addr, relocated_addr + image_size);
		memmove((void *)relocated_addr, (void *)ld, image_size);
	}

	images->ep = relocated_addr;
	images->os.start = relocated_addr;
	images->os.end = relocated_addr + image_size;

	lmb_reserve(&images->lmb, images->ep, le32_to_cpu(image_size));

	/*
	 * Handle the BOOTM_STATE_FINDOTHER state ourselves as we do not
	 * have a header that provide this informaiton.
	 */
	if (bootm_find_images(flag, argc, argv, relocated_addr, image_size))
		return 1;

	return 0;
}

int do_booti(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	int ret;

	/* Consume 'booti' */
	argc--; argv++;

	if (booti_start(cmdtp, flag, argc, argv, &images))
		return 1;

	/*
	 * We are doing the BOOTM_STATE_LOADOS state ourselves, so must
	 * disable interrupts ourselves
	 */
	bootm_disable_interrupts();

	images.os.os = IH_OS_LINUX;
#ifdef CONFIG_RISCV_SMODE
	images.os.arch = IH_ARCH_RISCV;
#elif CONFIG_ARM64
	images.os.arch = IH_ARCH_ARM64;
#endif
	ret = do_bootm_states(cmdtp, flag, argc, argv,
#ifdef CONFIG_SYS_BOOT_RAMDISK_HIGH
			      BOOTM_STATE_RAMDISK |
#endif
			      BOOTM_STATE_OS_PREP | BOOTM_STATE_OS_FAKE_GO |
			      BOOTM_STATE_OS_GO,
			      &images, 1);

	return ret;
}

#ifdef CONFIG_SYS_LONGHELP
static char booti_help_text[] =
	"[addr [initrd[:size]] [fdt]]\n"
	"    - boot Linux flat or compressed 'Image' stored at 'addr'\n"
	"\tThe argument 'initrd' is optional and specifies the address\n"
	"\tof an initrd in memory. The optional parameter ':size' allows\n"
	"\tspecifying the size of a RAW initrd.\n"
	"\tCurrently only booting from gz, bz2, lzma and lz4 compression\n"
	"\ttypes are supported. In order to boot from any of these compressed\n"
	"\timages, user have to set kernel_comp_addr_r and kernel_comp_size environment\n"
	"\tvariables beforehand.\n"
#if defined(CONFIG_OF_LIBFDT)
	"\tSince booting a Linux kernel requires a flat device-tree, a\n"
	"\tthird argument providing the address of the device-tree blob\n"
	"\tis required. To boot a kernel with a device-tree blob but\n"
	"\twithout an initrd image, use a '-' for the initrd argument.\n"
#endif
	"";
#endif

U_BOOT_CMD(
	booti,	CONFIG_SYS_MAXARGS,	1,	do_booti,
	"boot Linux kernel 'Image' format from memory", booti_help_text
);
