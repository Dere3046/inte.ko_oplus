// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2026 dere3046
 */

#include <linux/bitops.h>
#include <linux/elf.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>

#include "btf.h"
#include "core.h"
#include "type_info.h"

int inte_compat_init(void);
void inte_compat_exit(void);
unsigned long *inte_compat_sys_call_table(void);
int inte_compat_load_info(void *info, void **header, unsigned long *length);

static struct ti_resolver inte_ti_resolver;
static unsigned long *inte_syscall_table;
static bool inte_ti_ready;
static u32 inte_header_offset;
static u32 inte_length_offset;

static __nocfi noinline unsigned long inte_resolve_raw(const char *name)
{
	return kallrecon_klp(name);
}

static int inte_layout_probe(void)
{
	struct ti_ctx *ctx;
	u32 id, bit, size, header_offset, length_offset;

	ctx = ti_base();
	if (ti_type_by_name(ctx, "load_info", BIT(BTF_KIND_STRUCT), &id))
		return -ENOENT;
	if (ti_member_off(ctx, id, "hdr", &bit, &size))
		return -ENOENT;
	header_offset = bit / 8;
	if (ti_member_off(ctx, id, "len", &bit, &size))
		return -ENOENT;
	length_offset = bit / 8;

	inte_header_offset = header_offset;
	inte_length_offset = length_offset;
	return 0;
}

static bool inte_image_valid(unsigned long header, unsigned long length)
{
	u32 magic;

	if (!header || length < sizeof(Elf_Ehdr) || length > (1UL << 30))
		return false;
	if (safe_read(&magic, (const void *)header, sizeof(magic)))
		return false;

	return magic == 0x464c457f;
}

int inte_compat_init(void)
{
	int ret;

	find_kallsyms_base();
	if (!klnum_val || !kallrecon_klp) {
		pr_err("[inte_compat] kallsyms discovery failed\n");
		return -ENODATA;
	}

	inte_ti_resolver.name_to_addr = inte_resolve_raw;
	ret = ti_init(&inte_ti_resolver);
	if (ret)
		return -ENOTSUPP;
	inte_ti_ready = true;

	if (!ti_btf_available())
		goto no_layout;
	ret = inte_layout_probe();
	if (ret)
		goto no_layout;

	inte_syscall_table = (unsigned long *)inte_resolve_raw("sys_call_table");
	if (!inte_syscall_table) {
		pr_err("[inte_compat] sys_call_table not found\n");
		ret = -ENOENT;
		goto fail;
	}

	pr_info("[inte_compat] load_info hdr=%u len=%u\n",
		inte_header_offset, inte_length_offset);
	return 0;

no_layout:
	ret = -ENOTSUPP;
fail:
	inte_compat_exit();
	return ret;
}

void inte_compat_exit(void)
{
	if (!inte_ti_ready)
		return;

	ti_exit();
	inte_ti_ready = false;
}

unsigned long *inte_compat_sys_call_table(void)
{
	return inte_syscall_table;
}

int inte_compat_load_info(void *info, void **header, unsigned long *length)
{
	unsigned long image_header, image_length;

	if (!info || !header || !length)
		return -EINVAL;
	if (safe_read(&image_header, (const char *)info + inte_header_offset,
		      sizeof(image_header)))
		return -EFAULT;
	if (safe_read(&image_length, (const char *)info + inte_length_offset,
		      sizeof(image_length)))
		return -EFAULT;
	if (!inte_image_valid(image_header, image_length))
		return -EINVAL;

	*header = (void *)image_header;
	*length = image_length;
	return 0;
}
