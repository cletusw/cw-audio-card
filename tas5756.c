#include <linux/init.h>
#include <linux/kernel.h> /* Needed for pr_info() */
#include <linux/module.h>

MODULE_DESCRIPTION("Implementation of a TAS5756M driver for learning purposes");
MODULE_AUTHOR("Clayton Watts <cletusw@gmail.com>");
MODULE_LICENSE("Dual MIT/GPL");

static int __init tas5756_init(void) {
	/* To show logs: */
	/* sudo journalctl --since "5 minutes ago" | grep kernel */
	/* Or tail ("wait on") the logs with: */
	/* dmesg -w */
	/* (optionally use `-l info` for just info level logs) */
	pr_info("Hello, cletusw.\n");

	/* A non 0 return means init_module failed; module can't be loaded. */
	return 0;
}

static void __exit tas5756_exit(void) {
	pr_info("Goodbye, cletusw.\n");
}

module_init(tas5756_init);
module_exit(tas5756_exit);
