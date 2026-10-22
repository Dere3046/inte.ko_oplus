// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2026 dere3046
 */

#define POC_NAME "poc_bug_overread"
#define POC_DESC "inte poc malformed modinfo name"

#include "poc_common.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("dere3046");
MODULE_DESCRIPTION(POC_DESC);

static int __init poc_init(void)
{
	poc_info("loaded");
	return 0;
}

static void __exit poc_exit(void)
{
}

module_init(poc_init);
module_exit(poc_exit);
