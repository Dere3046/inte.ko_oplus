// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2026 dere3046
 */

#define POC_NAME "poc_gate_off"
#define POC_DESC "inte poc clear boot stage"

#include "poc_common.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("dere3046");
MODULE_DESCRIPTION(POC_DESC);

static u32 saved_gate;

static int __init poc_init(void)
{
	int ret;

	ret = poc_init_common();
	if (ret)
		return ret;
	if (!poc_boot_stage)
		return -EINVAL;
	saved_gate = READ_ONCE(*(u32 *)poc_boot_stage);
	poc_store_u32((void *)poc_boot_stage, 0);
	poc_info("gate %u -> 0", saved_gate);
	return 0;
}

static void __exit poc_exit(void)
{
	if (poc_boot_stage)
		poc_store_u32((void *)poc_boot_stage, saved_gate);
	poc_exit_common();
}

module_init(poc_init);
module_exit(poc_exit);
