#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/kernel.h> /* Needed for pr_info() */
#include <linux/module.h>
#include <linux/regmap.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/tlv.h>

MODULE_DESCRIPTION("Implementation of a TAS5756M driver for learning purposes");
MODULE_AUTHOR("Clayton Watts <cletusw@gmail.com>");
MODULE_LICENSE("Dual MIT/GPL");

// TAS5756 actually uses multiple pages of registers. Here we just assume page 0
#define TAS5756_CLOCK_MISSING_DETECTION_PERIOD 44
#define TAS5756_VOLL 61
#define TAS5756_VOLR 62

#define TAS5756_MAX_REGISTER TAS5756_VOLR
#define TAS5756_MAX_VOLUME_VALUE 0xFF

/* Private data for the TAS5756 */
struct tas5756_private {
	struct regmap *regmap;
};

static const DECLARE_TLV_DB_SCALE(volume_tlv, -10350, 50, 1 /* mute = true */);

static const struct snd_kcontrol_new tas5756_snd_controls[] = {
	SOC_DOUBLE_R_TLV(
			"Hardware Master Playback Volume",
			TAS5756_VOLL,
			TAS5756_VOLR,
			/* xshift = */ 0,
			TAS5756_MAX_VOLUME_VALUE,
			/* invert = */ 1,
			volume_tlv),
};

// Mux practice
static const char * const tas5756_fake_source_mux_text[] = {
	"Pi",
	"Aux",
};
static const struct soc_enum tas5756_fake_source_mux_enum =
	SOC_ENUM_SINGLE(
			TAS5756_CLOCK_MISSING_DETECTION_PERIOD,
			/* xshift = */ 0,
			ARRAY_SIZE(tas5756_fake_source_mux_text),
			tas5756_fake_source_mux_text);
static const struct snd_kcontrol_new tas5756_fake_source_mux =
	SOC_DAPM_ENUM("SOURCEMUX", tas5756_fake_source_mux_enum);

static int corgi_amp_event(struct snd_soc_dapm_widget *w, struct snd_kcontrol *k, int event) {
	pr_info("tas5756: amp DAPM event!");
	return 0;
}

static const struct snd_soc_dapm_widget tas5756_dapm_widgets[] = {
	SND_SOC_DAPM_INPUT("DACINL"),
	SND_SOC_DAPM_INPUT("DACINR"),
	SND_SOC_DAPM_INPUT("AUXINL"),
	SND_SOC_DAPM_INPUT("AUXINR"),

	SND_SOC_DAPM_MUX_E("Source select", SND_SOC_NOPM, 0, 0, &tas5756_fake_source_mux, corgi_amp_event, 0x3F),

	SND_SOC_DAPM_OUTPUT("MUXOUTL"),
	SND_SOC_DAPM_OUTPUT("MUXOUTR"),
};

static const struct snd_soc_dapm_route tas5756_routes[] = {
	{ "Source select", NULL, "DACINL" },
	{ "Source select", NULL, "DACINR" },
	{ "Source select", NULL, "AUXINL" },
	{ "Source select", NULL, "AUXINR" },

	{ "MUXOUTL", NULL, "Source select" },
	{ "MUXOUTR", NULL, "Source select" },
};

static const struct snd_soc_component_driver soc_component_dev_tas5756 = {
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

static bool tas5756_readable_register(struct device *dev, unsigned int reg)
{
	pr_info("tas5756: hw read %d", reg);

	switch (reg) {
	case TAS5756_CLOCK_MISSING_DETECTION_PERIOD ... TAS5756_VOLR:
		return true;
	default:
		return false;
	}
}

static bool tas5756_writeable_register(struct device *dev, unsigned int reg)
{
	pr_info("tas5756: hw write %d", reg);

	switch (reg) {
	case TAS5756_CLOCK_MISSING_DETECTION_PERIOD ... TAS5756_VOLR:
		return true;
	default:
		return false;
	}
}

static const struct regmap_config tas5756_regmap = {
	.reg_bits = 8,
	.val_bits = 8,

	.max_register = TAS5756_MAX_REGISTER,
	.readable_reg = tas5756_readable_register,
	.writeable_reg = tas5756_writeable_register,
	.cache_type = REGCACHE_NONE,
};

static int tas5756_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct tas5756_private *tas5756;
	int return_value;

	/* To tail ("wait on") the logs: */
	/* dmesg -w */
	/* (optionally use `-l info` for just info level logs) */
	pr_info("tas5756: Hello from driver v3!\n");

	tas5756 = devm_kzalloc(&client->dev, sizeof(*tas5756), GFP_KERNEL);
	if (!tas5756) {
		return -ENOMEM;
	}

	i2c_set_clientdata(client, tas5756);

	tas5756->regmap = devm_regmap_init_i2c(client, &tas5756_regmap);
	if (IS_ERR(tas5756->regmap)) {
		return_value = PTR_ERR(tas5756->regmap);
		pr_info("tas5756: regmap_init() failed with: %d\n", return_value);
		return return_value;
	}

	return devm_snd_soc_register_component(
			&client->dev, &soc_component_dev_tas5756, NULL, 0);
}

static int tas5756_i2c_remove(struct i2c_client *client)
{
	pr_info("tas5756: Goodbye!\n");

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
