// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2026 dere3046
 */

#define POC_NAME "poc_events_forge"
#define POC_DESC "inte poc forge ko events"

#include "poc_common.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("dere3046");
MODULE_DESCRIPTION(POC_DESC);

static int __init poc_init(void)
{
	char name[32];
	int i;
	int ret;

	ret = poc_init_common();
	if (ret)
		return ret;
	if (!poc_add_event)
		return -EINVAL;
	for (i = 0; i < 12; i++) {
		snprintf(name, sizeof(name), "fake_%02d.ko", i);
		poc_call_add_event((void *)poc_add_event, name);
	}
	poc_info("forged 12 events");
	return 0;
}

static void __exit poc_exit(void)
{
	poc_exit_common();
}

module_init(poc_init);
module_exit(poc_exit);
