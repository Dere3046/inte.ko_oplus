// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2026 dere3046
 */

#define POC_NAME "poc_bug_timeout"
#define POC_DESC "inte poc oversized module hash timeout"

#include "poc_common.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("dere3046");
MODULE_DESCRIPTION(POC_DESC);

static const char payload[64 * 1024 * 1024] __used = { 1 };

static int __init poc_init(void)
{
	poc_info("loaded size=%zu", sizeof(payload));
	return 0;
}

static void __exit poc_exit(void)
{
}

module_init(poc_init);
module_exit(poc_exit);
