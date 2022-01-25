/*
 * Copyright (C) 2022 Efinix Inc. All rights reserved.
 *
 * SPDX-License-Identifier: GPL-3.0
 *
 */
#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mohamd Noor Alim <mnalim@efinixinc.com");
MODULE_DESCRIPTION("Sample hello world kernel module");

static int hello_world_init(void)
{
	pr_info("Hello world\n");
	return 0;
}

static void hello_world_exit(void)
{
	pr_info("Bye\n");
}

module_init(hello_world_init);
module_exit(hello_world_exit);
