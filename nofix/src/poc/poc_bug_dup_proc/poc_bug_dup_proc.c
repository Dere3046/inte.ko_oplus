// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2026 dere3046
 */

#define POC_NAME "poc_bug_dup_proc"
#define POC_DESC "inte poc duplicate proc entry"

#include "poc_common.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("dere3046");
MODULE_DESCRIPTION(POC_DESC);

static struct proc_dir_entry *poc_entry;

static ssize_t dup_read(struct file *file, char __user *buf, size_t count,
			loff_t *ppos)
{
	return 0;
}

static const struct proc_ops dup_ops = {
	.proc_read = dup_read,
};

static int __init poc_init(void)
{
	int ret;

	ret = poc_init_common();
	if (ret)
		return ret;
	poc_entry = proc_create("inte_ko", 0666, NULL, &dup_ops);
	if (!poc_entry) {
		poc_err("proc_create failed");
		return -EEXIST;
	}
	poc_info("inte_ko occupied");
	return 0;
}

static void __exit poc_exit(void)
{
	proc_remove(poc_entry);
	poc_exit_common();
}

module_init(poc_init);
module_exit(poc_exit);
