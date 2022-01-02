#include <linux/init.h>
#include <linux/kernel.h> /* Needed for pr_info() */
#include <linux/module.h>

MODULE_DESCRIPTION("A sample driver");
MODULE_AUTHOR("Clayton Watts <cletusw@gmail.com>");
MODULE_LICENSE("Dual MIT/GPL");

static int __init my_init(void) {
	/* To show logs: */
	/* sudo journalctl --since "5 minutes ago" | grep kernel */
	/* Or tail ("wait on") the logs with: */
	/* dmesg -w */
	/* (optionally use `-l info` for just info level logs) */
	pr_info("Hello, cletusw.\n");

	/* A non 0 return means init_module failed; module can't be loaded. */
	return 0;
}

static void __exit my_exit(void) {
	pr_info("Goodbye, cletusw.\n");
}

module_init(my_init);
module_exit(my_exit);
