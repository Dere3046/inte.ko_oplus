// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2026 dere3046
 */

#include <linux/init.h>
#include <linux/module.h>

static int __init badmod_init(void)
{
	pr_info("[badmod] loaded\n");
	return 0;
}

static void __exit badmod_exit(void)
{
	pr_info("[badmod] unloaded\n");
}

module_init(badmod_init);
module_exit(badmod_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("unknown module for inte");
