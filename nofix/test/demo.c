// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2026 dere3046
 */

#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

static int read_file(const char *path, char *buf, size_t size)
{
	int fd = open(path, O_RDONLY);
	ssize_t n;

	if (fd < 0)
		return -errno;
	n = read(fd, buf, size - 1);
	close(fd);
	if (n < 0)
		return -errno;
	buf[n] = '\0';
	return (int)n;
}

static int load_module(const char *path, const char *params)
{
	int fd = open(path, O_RDONLY);
	int ret;

	if (fd < 0)
		return -errno;
	ret = syscall(SYS_finit_module, fd, params, 0);
	if (ret)
		ret = -errno;
	close(fd);
	return ret;
}

static int unload_module(const char *name)
{
	int ret;

	ret = syscall(SYS_delete_module, name, 0);
	return ret ? -errno : 0;
}

static int write_status(void)
{
	int one = 1;
	int fd = open("/proc/inte_status", O_WRONLY);
	ssize_t n;

	if (fd < 0)
		return -errno;
	n = write(fd, &one, sizeof(one));
	close(fd);
	return n == sizeof(one) ? 0 : -1;
}

static int check(void)
{
	char ko[4096] = { 0 };
	char systbl[4096] = { 0 };
	int ko_len;
	int systbl_len;
	int anomaly = 0;

	ko_len = read_file("/proc/inte_ko", ko, sizeof(ko));
	systbl_len = read_file("/proc/inte_systbl", systbl, sizeof(systbl));

	printf("[inte_demo] inte_ko(%d)=\"%s\"\n", ko_len, ko);
	printf("[inte_demo] inte_systbl(%d)=\"%s\"\n", systbl_len, systbl);

	if (ko_len < 0 || systbl_len < 0)
		anomaly = 1;
	if (ko_len > 0 && ko[0] != '\0')
		anomaly = 1;
	if (systbl_len > 0 && systbl[0] != '\0')
		anomaly = 1;

	printf("[inte_demo] verdict=%s\n", anomaly ? "ANOMALY" : "CLEAN");
	return anomaly;
}

static int demo(const char *dir)
{
	char path[512];
	char modules[4096];
	int ret;
	int verdict;
	int ok = 1;

	snprintf(path, sizeof(path), "%s/inte.ko", dir);
	ret = load_module(path, "");
	printf("[inte_demo] load inte: %d\n", ret);
	if (ret)
		return 1;

	ret = write_status();
	printf("[inte_demo] status: %d\n", ret);
	if (ret)
		return 1;

	snprintf(path, sizeof(path), "%s/badmod.ko", dir);
	ret = load_module(path, "");
	printf("[inte_demo] load badmod: %d\n", ret);
	if (ret)
		return 1;

	printf("[inte_demo] check before hide\n");
	verdict = check();
	printf("[inte_demo] before=%d\n", verdict);
	if (verdict != 1)
		ok = 0;

	snprintf(path, sizeof(path), "%s/poc_hide.ko", dir);
	ret = load_module(path, "");
	printf("[inte_demo] load hide: %d\n", ret);
	if (ret)
		ok = 0;

	printf("[inte_demo] check after hide\n");
	verdict = check();
	printf("[inte_demo] after=%d\n", verdict);
	if (verdict != 0)
		ok = 0;

	if (read_file("/proc/modules", modules, sizeof(modules)) > 0)
		printf("[inte_demo] badmod_still_loaded=%d\n",
		       strstr(modules, "badmod") != NULL);

	ret = unload_module("poc_hide");
	printf("[inte_demo] unload hide: %d\n", ret);
	if (ret)
		ok = 0;

	printf("[inte_demo] check control\n");
	verdict = check();
	printf("[inte_demo] control=%d\n", verdict);
	if (verdict != 1)
		ok = 0;

	printf("[inte_demo] cleanup\n");
	ret = unload_module("badmod");
	printf("[inte_demo] unload badmod: %d\n", ret);
	if (ret)
		ok = 0;
	ret = unload_module("inte");
	printf("[inte_demo] unload inte: %d\n", ret);
	if (ret)
		ok = 0;

	printf("[inte_demo] %s\n", ok ? "PASS" : "FAIL");
	return ok ? 0 : 1;
}

static void usage(const char *prog)
{
	printf("usage:\n");
	printf("  %s check\n", prog);
	printf("  %s status\n", prog);
	printf("  %s load <module.ko> [params]\n", prog);
	printf("  %s unload <name>\n", prog);
	printf("  %s demo [dir]\n", prog);
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		usage(argv[0]);
		return 2;
	}

	if (strcmp(argv[1], "check") == 0)
		return check();
	if (strcmp(argv[1], "status") == 0) {
		int ret = write_status();

		printf("[inte_demo] status: %d\n", ret);
		return ret ? 1 : 0;
	}
	if (strcmp(argv[1], "load") == 0) {
		int ret;

		if (argc < 3) {
			usage(argv[0]);
			return 2;
		}
		ret = load_module(argv[2], argc > 3 ? argv[3] : "");
		printf("[inte_demo] load %s: %d\n", argv[2], ret);
		return ret ? 1 : 0;
	}
	if (strcmp(argv[1], "unload") == 0) {
		int ret;

		if (argc < 3) {
			usage(argv[0]);
			return 2;
		}
		ret = unload_module(argv[2]);
		printf("[inte_demo] unload %s: %d\n", argv[2], ret);
		return ret ? 1 : 0;
	}
	if (strcmp(argv[1], "demo") == 0)
		return demo(argc > 2 ? argv[2] : ".");

	usage(argv[0]);
	return 2;
}
