// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2026 dere3046
 */

#define POC_NAME "poc_write_probe"
#define POC_DESC "inte poc write route probe"

#include "poc_common.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("dere3046");
MODULE_DESCRIPTION(POC_DESC);
#include <asm/unistd.h>

static void probe_word(const char *name, void *addr, bool read_only)
{
	unsigned long orig = READ_ONCE(*(unsigned long *)addr);
	unsigned long flipped = orig ^ 1UL;
	unsigned long readback;
	int ret;

	if (read_only)
		ret = poc_patch_word(addr, flipped);
	else {
		poc_store_word(addr, flipped);
		ret = 0;
	}

	if (ret) {
		poc_info("%s write failed %d", name, ret);
		return;
	}

	readback = READ_ONCE(*(unsigned long *)addr);
	poc_info("%s %s orig=%lx read=%lx", name,
		 readback == flipped ? "LANDS" : "DROPPED", orig, readback);

	if (read_only)
		poc_patch_word(addr, orig);
	else
		poc_store_word(addr, orig);
}

static void probe_gate(void)
{
	u32 orig = READ_ONCE(*(u32 *)poc_boot_stage);

	poc_store_u32((void *)poc_boot_stage, orig ^ 1U);
	poc_info("gate %s orig=%u read=%u",
		 READ_ONCE(*(u32 *)poc_boot_stage) == (orig ^ 1U) ? "LANDS" : "DROPPED",
		 orig, READ_ONCE(*(u32 *)poc_boot_stage));
	poc_store_u32((void *)poc_boot_stage, orig);
}

static void probe_table(void)
{
	unsigned long *table;
	unsigned long entry;
	unsigned long symbol;

	table = (unsigned long *)poc_resolve("sys_call_table");
	if (!table) {
		poc_err("sys_call_table unresolved");
		return;
	}
	entry = READ_ONCE(table[__NR_getpid]);
	symbol = poc_resolve("__arm64_sys_getpid");
	poc_info("table[getpid]=%lx symbol=%lx %s", entry, symbol,
		 entry == symbol ? "UNPATCHED" : "PATCHED");
}

static int __init poc_init(void)
{
	int ret;

	ret = poc_init_common();
	if (ret)
		return ret;
	if (poc_boot_stage)
		probe_gate();
	if (poc_hash_baseline)
		probe_word("baseline", (void *)poc_hash_baseline, true);
	probe_table();
	return 0;
}

static void __exit poc_exit(void)
{
	poc_exit_common();
}

module_init(poc_init);
module_exit(poc_exit);
