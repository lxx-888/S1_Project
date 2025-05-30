// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (C) 2017 The Android Open Source Project
 */
#include <common.h>
#include <android_ab.h>
#include <android_bootloader_message.h>
#include <blk.h>
#include <log.h>
#include <malloc.h>
#include <part.h>
#include <memalign.h>
#include <linux/err.h>
#include <u-boot/crc.h>
#include <u-boot/crc.h>

#ifdef CONFIG_SSTAR_ANDROID_AB
const char *SLOT_SUFFIX[] = {"_a", "_b"};
#endif

/**
 * Compute the CRC-32 of the bootloader control struct.
 *
 * Only the bytes up to the crc32_le field are considered for the CRC-32
 * calculation.
 *
 * @param[in] abc bootloader control block
 *
 * @return crc32 sum
 */
uint32_t ab_control_compute_crc(struct bootloader_control *abc)
{
	return crc32(0, (void *)abc, offsetof(typeof(*abc), crc32_le));
}

/**
 * Initialize bootloader_control to the default value.
 *
 * It allows us to boot all slots in order from the first one. This value
 * should be used when the bootloader message is corrupted, but not when
 * a valid message indicates that all slots are unbootable.
 *
 * @param[in] abc bootloader control block
 *
 * @return 0 on success and a negative on error
 */
int ab_control_default(struct bootloader_control *abc)
{
	int i;
	const struct slot_metadata metadata = {
		.priority = 15,
		.tries_remaining = 3,
		.successful_boot = 0,
		.verity_corrupted = 0,
		.reserved = 0
	};

	if (!abc)
		return -EFAULT;

	memcpy(abc->slot_suffix, "_a\0\0", 4);
	abc->magic = BOOT_CTRL_MAGIC;
	abc->version = BOOT_CTRL_VERSION;
	abc->nb_slot = NUM_SLOTS;
	memset(abc->reserved0, 0, sizeof(abc->reserved0));
	for (i = 0; i < abc->nb_slot; ++i) {
		abc->slot_info[i] = metadata;

		/* One slot should always have higher priority than other slots,
		 * otherwise we can ping-pong between slots based on tries_remaining.
		 * Since the default slot is _a, make _a highest priority.
		 */
		if (i != 0)
			abc->slot_info[i].priority = metadata.priority - 1;
	}

	memset(abc->reserved1, 0, sizeof(abc->reserved1));
	abc->crc32_le = ab_control_compute_crc(abc);

	return 0;
}

/**
 * Load the boot_control struct from disk into newly allocated memory.
 *
 * This function allocates and returns an integer number of disk blocks,
 * based on the block size of the passed device to help performing a
 * read-modify-write operation on the boot_control struct.
 * The boot_control struct offset (2 KiB) must be a multiple of the device
 * block size, for simplicity.
 *
 * @param[in] dev_desc Device where to read the boot_control struct from
 * @param[in] part_info Partition in 'dev_desc' where to read from, normally
 *			the "misc" partition should be used
 * @param[out] pointer to pointer to bootloader_control data
 * @return 0 on success and a negative on error
 */
int ab_control_create_from_disk(struct blk_desc *dev_desc,
				       const struct disk_partition *part_info,
				       struct bootloader_control **abc)
{
	ulong abc_offset, abc_blocks, ret;

	abc_offset = offsetof(struct bootloader_message_ab, slot_suffix);
	if (abc_offset % part_info->blksz) {
		log_err("ANDROID: Boot control block not block aligned.\n");
		return -EINVAL;
	}
	abc_offset /= part_info->blksz;

	abc_blocks = DIV_ROUND_UP(sizeof(struct bootloader_control),
				  part_info->blksz);
	if (abc_offset + abc_blocks > part_info->size) {
		log_err("ANDROID: boot control partition too small. Need at");
		log_err(" least %lu blocks but have %llu blocks.\n",
			abc_offset + abc_blocks,
			(unsigned long long)part_info->size);
		return -EINVAL;
	}
	*abc = malloc_cache_aligned(abc_blocks * part_info->blksz);
	if (!*abc)
		return -ENOMEM;

	ret = blk_dread(dev_desc, part_info->start + abc_offset, abc_blocks,
			*abc);
	if (IS_ERR_VALUE(ret)) {
		log_err("ANDROID: Could not read from boot ctrl partition\n");
		free(*abc);
		return -EIO;
	}

	log_debug("ANDROID: Loaded ABC, %lu blocks\n", abc_blocks);

#ifdef CONFIG_SSTAR_ANDROID_AB
	extern int ab_control_store(struct blk_desc *dev_desc,
			const struct disk_partition *part_info,
			struct bootloader_control *abc);
	if ((*abc)->crc32_le != ab_control_compute_crc(*abc)) {
		printf("A/B metadata incomplete,re-initializing.\n");
		if (ab_control_default(*abc) < 0) {
			return -EINVAL;
		}
		(*abc)->crc32_le = ab_control_compute_crc(*abc);
		return ab_control_store(dev_desc, part_info, *abc);
	}
#endif

	return 0;
}

/**
 * Store the loaded boot_control block.
 *
 * Store back to the same location it was read from with
 * ab_control_create_from_misc().
 *
 * @param[in] dev_desc Device where we should write the boot_control struct
 * @param[in] part_info Partition on the 'dev_desc' where to write
 * @param[in] abc Pointer to the boot control struct and the extra bytes after
 *                it up to the nearest block boundary
 * @return 0 on success and a negative on error
 */
int ab_control_store(struct blk_desc *dev_desc,
			    const struct disk_partition *part_info,
			    struct bootloader_control *abc)
{
	ulong abc_offset, abc_blocks, ret;

	abc_offset = offsetof(struct bootloader_message_ab, slot_suffix) /
		     part_info->blksz;
	abc_blocks = DIV_ROUND_UP(sizeof(struct bootloader_control),
				  part_info->blksz);
	ret = blk_dwrite(dev_desc, part_info->start + abc_offset, abc_blocks,
			 abc);
	if (IS_ERR_VALUE(ret)) {
		log_err("ANDROID: Could not write back the misc partition\n");
		return -EIO;
	}

	return 0;
}

static bool is_slot_bootable(const struct slot_metadata* slot)
{
	return slot->tries_remaining > 0 || slot->successful_boot;
}

/**
 * Compare two slots.
 *
 * The function determines slot which is should we boot from among the two.
 *
 * @param[in] a The first bootable slot metadata
 * @param[in] b The second bootable slot metadata
 * @return Negative if the slot "a" is better, positive of the slot "b" is
 *         better or 0 if they are equally good.
 */
static int ab_compare_slots(const struct slot_metadata *a,
			    const struct slot_metadata *b)
{
	/* Pick the highest priority slot that can be considered bootable. */
	if (a->priority > b->priority && is_slot_bootable(a))
		return -1;
	if (b->priority > a->priority && is_slot_bootable(b))
		return 1;

	/* The higher priority slot is not bootable, so pick the slot that is. */
	return b->successful_boot - a->successful_boot;
}

int ab_select_slot(struct blk_desc *dev_desc, const struct disk_partition *part_info,
				   bool normal_boot)
{
	struct bootloader_control *abc = NULL;
#ifndef CONFIG_SSTAR_ANDROID_AB
	u32 crc32_le;
#endif
	int slot, i, ret;
	bool store_needed = false;
	char slot_suffix[4];

	ret = ab_control_create_from_disk(dev_desc, part_info, &abc);
	if (ret < 0) {
		/*
		 * This condition represents an actual problem with the code or
		 * the board setup, like an invalid partition information.
		 * Signal a repair mode and do not try to boot from either slot.
		 */
		return ret;
	}

#ifndef CONFIG_SSTAR_ANDROID_AB
	/* we already move CRC32 verify to ab_control_create_from_disk */
	crc32_le = ab_control_compute_crc(abc);
	if (abc->crc32_le != crc32_le) {
		log_err("ANDROID: Invalid CRC-32 (expected %.8x, found %.8x),",
			crc32_le, abc->crc32_le);
		log_err("re-initializing A/B metadata.\n");

		ret = ab_control_default(abc);
		if (ret < 0) {
			free(abc);
			return -ENODATA;
		}
		store_needed = true;
	}
#endif

	if (abc->magic != BOOT_CTRL_MAGIC) {
		log_err("ANDROID: Unknown A/B metadata: %.8x\n", abc->magic);
		free(abc);
		return -ENODATA;
	}

	if (abc->version > BOOT_CTRL_VERSION) {
		log_err("ANDROID: Unsupported A/B metadata version: %.8x\n",
			abc->version);
		free(abc);
		return -ENODATA;
	}

	/*
	 * At this point a valid boot control metadata is stored in abc,
	 * followed by other reserved data in the same block. We select a with
	 * the higher priority slot that
	 *  - is not marked as corrupted and
	 *  - either has tries_remaining > 0 or successful_boot is true.
	 * If the selected slot has a false successful_boot, we also decrement
	 * the tries_remaining until it eventually becomes unbootable because
	 * tries_remaining reaches 0. This mechanism produces a bootloader
	 * induced rollback, typically right after a failed update.
	 */

	/* Safety check: limit the number of slots. */
	if (abc->nb_slot > ARRAY_SIZE(abc->slot_info)) {
		abc->nb_slot = ARRAY_SIZE(abc->slot_info);
		store_needed = true;
	}

	slot = -1;
	for (i = 0; i < abc->nb_slot; ++i) {
		if (abc->slot_info[i].verity_corrupted ||
		    !abc->slot_info[i].tries_remaining) {
			log_debug("ANDROID: unbootable slot %d tries: %d, ",
				  i, abc->slot_info[i].tries_remaining);
			log_debug("corrupt: %d\n",
				  abc->slot_info[i].verity_corrupted);
			continue;
		}
		log_debug("ANDROID: bootable slot %d pri: %d, tries: %d, ",
			  i, abc->slot_info[i].priority,
			  abc->slot_info[i].tries_remaining);
		log_debug("corrupt: %d, successful: %d\n",
			  abc->slot_info[i].verity_corrupted,
			  abc->slot_info[i].successful_boot);

		if (slot < 0 ||
		    ab_compare_slots(&abc->slot_info[i],
				     &abc->slot_info[slot]) < 0) {
			slot = i;
		}
	}

	/* Fail to boot normally if there is no bootable slot. */
	if (normal_boot && !is_slot_bootable(&abc->slot_info[slot])) {
		log_err("ANDROID: No bootable slot was found.\n");
		return -EINVAL;
	}

	/* Note that we only count the boot attempt as a valid try when performing
	 * normal boots to Android. Booting to recovery or fastboot does not count
	 * as a normal boot.
	 */
	if (slot >= 0 && !abc->slot_info[slot].successful_boot && normal_boot) {
		log_err("ANDROID: Attempting slot %c, tries remaining %d\n",
			BOOT_SLOT_NAME(slot),
			abc->slot_info[slot].tries_remaining);
		abc->slot_info[slot].tries_remaining--;
		store_needed = true;
	}

	if (slot >= 0) {
		/*
		 * Legacy user-space requires this field to be set in the BCB.
		 * Newer releases load this slot suffix from the command line
		 * or the device tree.
		 */
		memset(slot_suffix, 0, sizeof(slot_suffix));
		slot_suffix[0] = '_';
		slot_suffix[1] = BOOT_SLOT_NAME(slot);
		if (memcmp(abc->slot_suffix, slot_suffix,
			   sizeof(slot_suffix))) {
			memcpy(abc->slot_suffix, slot_suffix,
			       sizeof(slot_suffix));
			store_needed = true;
		}
	}

	if (store_needed) {
		abc->crc32_le = ab_control_compute_crc(abc);
		ab_control_store(dev_desc, part_info, abc);
	}
	free(abc);

	if (slot < 0)
		return -EINVAL;

	return slot;
}

#ifdef CONFIG_SSTAR_ANDROID_AB
int ab_mark_current_slot_unbootable(struct blk_desc *dev_desc, const struct disk_partition *part_info)
{
	struct bootloader_control *abc = NULL;
	int slot_num, ret;

	ret = ab_control_create_from_disk(dev_desc, part_info, &abc);
	if (ret < 0) {
		return ret;
	}

	slot_num = abc->slot_suffix[1] - 'a';
	abc->slot_info[slot_num].priority = 0;
	abc->slot_info[slot_num].tries_remaining = 0;
	abc->slot_info[slot_num].successful_boot = 0;
	abc->crc32_le = ab_control_compute_crc(abc);
	ab_control_store(dev_desc, part_info, abc);

	free(abc);
	return 0;
}

bool ab_has_current_slot_boot_successful(struct blk_desc *dev_desc, const struct disk_partition *part_info)
{
	struct bootloader_control *abc = NULL;
	int slot_num, boot_successful;
	int ret;

	ret = ab_control_create_from_disk(dev_desc, part_info, &abc);
	if (ret < 0) {
		return -1;
	}

	slot_num = abc->slot_suffix[1] - 'a';
	boot_successful = abc->slot_info[slot_num].successful_boot;

	free(abc);
	return boot_successful;
}

const char *ab_get_current_slot_suffix(struct blk_desc *dev_desc, const struct disk_partition *part_info)
{
	struct bootloader_control *abc = NULL;
	int slot = -1;

	if (ab_control_create_from_disk(dev_desc, part_info, &abc) < 0) {
		log_err("ANDROID: Get boot control block fail\n");
		return NULL;
	}

	if (abc->magic != BOOT_CTRL_MAGIC) {
		log_err("ANDROID: Unknown A/B metadata: %.8x\n", abc->magic);
		goto out;
	}

	if (abc->version > BOOT_CTRL_VERSION) {
		log_err("ANDROID: Unsupported A/B metadata version: %.8x\n", abc->version);
		goto out;
	}

	for (int i = 0; i < abc->nb_slot; ++i) {
		if (abc->slot_info[i].verity_corrupted ||
		    !abc->slot_info[i].tries_remaining) {
			continue;
		}

		if (slot < 0 ||
		    ab_compare_slots(&abc->slot_info[i],
				     &abc->slot_info[slot]) < 0) {
			slot = i;
		}
	}

	/* Fail to boot normally if there is no bootable slot. */
	if (!is_slot_bootable(&abc->slot_info[slot])) {
		log_err("ANDROID: No bootable slot.\n");
		goto out;
	}

	log_debug("ANDROID: bootable slot %d pri: %d, tries: %d, corrupt: %d, successful: %d\n",
			slot, abc->slot_info[slot].priority,
			abc->slot_info[slot].tries_remaining,
			abc->slot_info[slot].verity_corrupted,
			abc->slot_info[slot].successful_boot);

out:
	free(abc);
	return (slot == -1) ? NULL : SLOT_SUFFIX[slot];
}
#endif

