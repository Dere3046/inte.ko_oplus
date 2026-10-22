// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2026 dere3046
 */

#ifndef POC_COMMON_H
#define POC_COMMON_H

#include <linux/bitops.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/workqueue.h>

#include "btf.h"
#include "core.h"
#include "hk.h"
#include "hk_patch.h"
#include "hk_ptr.h"
#include "type_info.h"

extern unsigned long poc_sha256_tfm;
extern unsigned long poc_boot_stage;
extern unsigned long poc_hash_probe;
extern unsigned long poc_entry_function;
extern unsigned long poc_hash_baseline;
extern unsigned long poc_syscall_table;
extern unsigned long poc_check_task;
extern unsigned long poc_do_hash;
extern unsigned long poc_add_event;
extern unsigned long poc_find_hash;
extern unsigned long poc_hash_table;
extern unsigned long poc_hash_table_lock;
extern unsigned long poc_proc_ko_entry;
extern unsigned long poc_proc_syscall_table_entry;
extern unsigned long poc_syscall_table_event_count;
extern char *poc_fake_hash;
extern char *poc_target_name;

unsigned long poc_resolve_symbol(const char *name);
int poc_init_common(void);
void poc_exit_common(void);
int poc_ti_offset(const char *type, const char *member, u32 *offset);
void poc_store_word(void *addr, unsigned long value);
void poc_store_u32(void *addr, u32 value);
int poc_patch_word(void *addr, unsigned long value);
int poc_proc_slot(unsigned long var_addr, void ***slot);
int poc_hex_to_bytes(const char *hex, u8 *out, size_t len);
void poc_call_check_task(void *addr);
int poc_call_do_hash(void *addr, unsigned long *data, u32 len, u8 *hash);
int poc_call_add_event(void *addr, const char *name);
unsigned char *poc_call_find_hash(void *addr, const char *name);

#ifndef POC_NAME
#define POC_NAME "poc"
#endif
#ifndef POC_DESC
#define POC_DESC POC_NAME
#endif

#define poc_info(fmt, ...) pr_info("[" POC_NAME "] " fmt, ##__VA_ARGS__)
#define poc_err(fmt, ...) pr_err("[" POC_NAME "] " fmt, ##__VA_ARGS__)

#endif
