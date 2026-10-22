// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2026 dere3046
 */

#define POC_NAME "poc_patch_text"
#define POC_DESC "inte poc patch hash entry to ret"

#include "poc_common.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("dere3046");
MODULE_DESCRIPTION(POC_DESC);

static u32 saved;

static int __init poc_init(void)
{
	u32 patch = 0xd65f03c0;
	int ret;

	ret = poc_init_common();
	if (ret)
		return ret;
	if (!poc_entry_function)
		return -EINVAL;
	memcpy(&saved, (void *)poc_entry_function, sizeof(saved));
	ret = hk_write_text((void *)poc_entry_function, &patch, sizeof(patch));
	if (ret) {
		poc_err("patch failed %d", ret);
		return ret;
	}
	poc_info("entry %08x -> %08x", saved, patch);
	return 0;
}

static void __exit poc_exit(void)
{
	if (poc_entry_function)
		hk_write_text((void *)poc_entry_function, &saved, sizeof(saved));
	poc_exit_common();
}

module_init(poc_init);
module_exit(poc_exit);
