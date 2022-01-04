#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/kernel.h> /* Needed for pr_info() */
#include <linux/module.h>
#include <sound/soc.h>

MODULE_DESCRIPTION("Implementation of a TAS5756M driver for learning purposes");
MODULE_AUTHOR("Clayton Watts <cletusw@gmail.com>");
MODULE_LICENSE("Dual MIT/GPL");

#define TAS5756_VOLB 61
#define TAS5756_VOLA 62

#define MAX_VOLUME_VALUE 0xFE

/* Private data for the TAS5756 */
struct tas5756_private {
	unsigned int cool_data;
};

static inline unsigned int tas5756_read(struct snd_soc_component *component,
	unsigned int reg)
{
	struct tas5756_private *tas5756 = snd_soc_component_get_drvdata(component);

	pr_info("tas5756: hw read %d value 0x%x\n", reg, tas5756->cool_data);

	return tas5756->cool_data;
}

static int tas5756_write(struct snd_soc_component *component, unsigned int reg,
		unsigned int value)
{
	struct tas5756_private *tas5756 = snd_soc_component_get_drvdata(component);

	pr_info("tas5756: hw write %d value 0x%x\n", reg, value);

	tas5756->cool_data = value;

	return 0;
}

static const struct snd_kcontrol_new tas5756_snd_controls[] = {
	SOC_SINGLE("Hardware Master Playback Volume", TAS5756_VOLB, 0, MAX_VOLUME_VALUE, 1),
};

static const struct snd_soc_dapm_widget tas5756_dapm_widgets[] = {
};

static const struct snd_soc_dapm_route tas5756_routes[] = {
};

static const struct snd_soc_component_driver soc_component_dev_tas5756 = {
	.write = tas5756_write,
	.read = tas5756_read,
	.controls = tas5756_snd_controls,
	.num_controls = ARRAY_SIZE(tas5756_snd_controls),
	.dapm_widgets = tas5756_dapm_widgets,
	.num_dapm_widgets = ARRAY_SIZE(tas5756_dapm_widgets),
	.dapm_routes = tas5756_routes,
	.num_dapm_routes = ARRAY_SIZE(tas5756_routes),
	.idle_bias_on = 1,
	.use_pmdown_time= 1,
	.endianness = 1,
	.non_legacy_dai_naming = 1,
};

static int tas5756_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct tas5756_private *tas5756;

	/* To show logs: */
	/* sudo journalctl --since "5 minutes ago" | grep kernel */
	/* Or tail ("wait on") the logs with: */
	/* dmesg -w */
	/* (optionally use `-l info` for just info level logs) */
	pr_info("tas5756: Hello from driver v3, cletusw!\n");

	tas5756 = devm_kzalloc(&client->dev, sizeof(*tas5756), GFP_KERNEL);
	if (!tas5756)
		return -ENOMEM;

	i2c_set_clientdata(client, tas5756);

	tas5756->cool_data = 3;

	return devm_snd_soc_register_component(
			&client->dev, &soc_component_dev_tas5756, NULL, 0);
}

static int tas5756_i2c_remove(struct i2c_client *client)
{
	pr_info("tas5756: Goodbye, cletusw.\n");

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
