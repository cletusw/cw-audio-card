//#include <linux/init.h>
#include <linux/kernel.h> /* Needed for pr_info() */
#include <linux/module.h>

MODULE_DESCRIPTION("A sample driver");
MODULE_AUTHOR("Clayton Watts <cletusw@gmail.com>");
MODULE_LICENSE("Dual MIT/GPL");

int init_module(void) {
	/* To show logs: */
	/* sudo journalctl --since "5 minutes ago" | grep kernel */
	/* Or tail ("wait on") just the info logs with: */
	/* dmesg -w -l info */
	pr_info("Hello, cletusw.\n");

	/* A non 0 return means init_module failed; module can't be loaded. */
	return 0;
}

void cleanup_module(void) {
	pr_info("Goodbye, cletusw.\n");
}
