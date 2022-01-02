#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/kernel.h> /* Needed for pr_info() */
#include <linux/module.h>

MODULE_DESCRIPTION("Implementation of a TAS5756M driver for learning purposes");
MODULE_AUTHOR("Clayton Watts <cletusw@gmail.com>");
MODULE_LICENSE("Dual MIT/GPL");

/* Private data for the TAS5756 */
struct tas5756_private {
	int cool_data;
};

static int tas5756_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct tas5756_private *tas5756;

	/* To show logs: */
	/* sudo journalctl --since "5 minutes ago" | grep kernel */
	/* Or tail ("wait on") the logs with: */
	/* dmesg -w */
	/* (optionally use `-l info` for just info level logs) */
	pr_info("Hello from driver v1, cletusw!\n");

	tas5756 = devm_kzalloc(&client->dev, sizeof(*tas5756), GFP_KERNEL);
	if (!tas5756)
		return -ENOMEM;

	i2c_set_clientdata(client, tas5756);

	return 0;
}

static int tas5756_i2c_remove(struct i2c_client *client)
{
	pr_info("Goodbye, cletusw.\n");

	return 0;
}

static const struct of_device_id tas5756_of_match[] = {
	{ .compatible = "ti,tas5756", },
	{},
};
MODULE_DEVICE_TABLE(of, tas5756_of_match);

static const struct i2c_device_id tas5756_i2c_id[] = {
	{"tas5756", 0},
	{}
};
MODULE_DEVICE_TABLE(i2c, tas5756_i2c_id);

static struct i2c_driver tas5756_i2c_driver = {
	.driver = {
		.name		= "tas5756",
		.of_match_table	= tas5756_of_match,
	},
	.id_table	= tas5756_i2c_id,
	.probe		= tas5756_i2c_probe,
	.remove		= tas5756_i2c_remove,
};

module_i2c_driver(tas5756_i2c_driver);
