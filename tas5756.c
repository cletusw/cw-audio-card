#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/kernel.h> /* Needed for pr_info() */
#include <linux/module.h>
#include <linux/regmap.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/tlv.h>

MODULE_DESCRIPTION("Implementation of a partial BD6490FV driver");
MODULE_AUTHOR("Clayton Watts <cletusw@gmail.com>");
MODULE_LICENSE("Dual MIT/GPL");

// BD6490FV actually uses multiple pages of registers. Here we just assume page 0
#define BD6490FV_CLOCK_MISSING_DETECTION_PERIOD 44
#define BD6490FV_VOLL 61
#define BD6490FV_VOLR 62

#define BD6490FV_MAX_REGISTER BD6490FV_VOLR
#define BD6490FV_MAX_VOLUME_VALUE 0xFF

/* Private data for the BD6490FV */
struct bd6490fv_private {
	struct regmap *regmap;
};

static const DECLARE_TLV_DB_SCALE(volume_tlv, -10350, 50, 1 /* mute = true */);

static const struct snd_kcontrol_new bd6490fv_snd_controls[] = {
	SOC_DOUBLE_R_TLV(
			"Hardware Master Playback Volume",
			BD6490FV_VOLL,
			BD6490FV_VOLR,
			/* xshift = */ 0,
			BD6490FV_MAX_VOLUME_VALUE,
			/* invert = */ 1,
			volume_tlv),
};

// Mux practice
static const char * const bd6490fv_fake_source_mux_text[] = {
	"Pi",
	"Aux",
};
static const struct soc_enum bd6490fv_fake_source_mux_enum =
	SOC_ENUM_SINGLE(
			BD6490FV_CLOCK_MISSING_DETECTION_PERIOD,
			/* xshift = */ 0,
			ARRAY_SIZE(bd6490fv_fake_source_mux_text),
			bd6490fv_fake_source_mux_text);
static const struct snd_kcontrol_new bd6490fv_fake_source_mux =
	SOC_DAPM_ENUM("SOURCEMUX", bd6490fv_fake_source_mux_enum);

static int bd6490fv_dapm_event(struct snd_soc_dapm_widget *w, struct snd_kcontrol *k, int event) {
	// pr_info("bd6490fv: pre DAPM event: %d", event);
	struct bd6490fv_private *bd6490fv = snd_soc_component_get_drvdata(w->dapm->component);
	int ret = 0;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		pr_info("bd6490fv: Change mux to DAC");
		// pr_info("bd6490fv: To DAC priv %p, regmap %p\n", (void*)bd6490fv, (void*)bd6490fv->regmap);
		ret = regmap_write(bd6490fv->regmap, BD6490FV_CLOCK_MISSING_DETECTION_PERIOD, 4);
		break;
	case SND_SOC_DAPM_PRE_PMD:
		pr_info("bd6490fv: Change mux to AUX");
		// pr_info("bd6490fv: To AUX priv %p, regmap %p\n", (void*)bd6490fv, (void*)bd6490fv->regmap);
		ret = regmap_write(bd6490fv->regmap, BD6490FV_CLOCK_MISSING_DETECTION_PERIOD, 5);
		break;
	}
	if (ret < 0) {
		pr_info("bd6490fv: Error changing mux: %d", ret);
		return -EIO;
	}
	return 0;
}

static const struct snd_soc_dapm_widget bd6490fv_dapm_widgets[] = {
	SND_SOC_DAPM_INPUT("DACINL"),
	SND_SOC_DAPM_INPUT("DACINR"),
	SND_SOC_DAPM_INPUT("AUXINL"),
	SND_SOC_DAPM_INPUT("AUXINR"),

	SND_SOC_DAPM_PRE("DAC DAPM listener", bd6490fv_dapm_event),
	SND_SOC_DAPM_MUX("Source select", SND_SOC_NOPM, 0, 0, &bd6490fv_fake_source_mux),

	SND_SOC_DAPM_OUTPUT("MUXOUTL"),
	SND_SOC_DAPM_OUTPUT("MUXOUTR"),
};

static const struct snd_soc_dapm_route bd6490fv_routes[] = {
	{ "DAC DAPM listener", NULL, "DACINL" },
	{ "DAC DAPM listener", NULL, "DACINR" },
	{ "Source select", NULL, "DAC DAPM listener" },
	{ "Source select", NULL, "AUXINL" },
	{ "Source select", NULL, "AUXINR" },

	{ "MUXOUTL", NULL, "Source select" },
	{ "MUXOUTR", NULL, "Source select" },
};

static const struct snd_soc_component_driver soc_component_dev_bd6490fv = {
	.controls = bd6490fv_snd_controls,
	.num_controls = ARRAY_SIZE(bd6490fv_snd_controls),
	.dapm_widgets = bd6490fv_dapm_widgets,
	.num_dapm_widgets = ARRAY_SIZE(bd6490fv_dapm_widgets),
	.dapm_routes = bd6490fv_routes,
	.num_dapm_routes = ARRAY_SIZE(bd6490fv_routes),
	.idle_bias_on = 1,
	.use_pmdown_time= 1,
	.endianness = 1,
	.non_legacy_dai_naming = 1,
};

static bool bd6490fv_readable_register(struct device *dev, unsigned int reg)
{
	pr_info("bd6490fv: hw read %d", reg);

	switch (reg) {
	case BD6490FV_CLOCK_MISSING_DETECTION_PERIOD ... BD6490FV_VOLR:
		return true;
	default:
		return false;
	}
}

static bool bd6490fv_writeable_register(struct device *dev, unsigned int reg)
{
	pr_info("bd6490fv: hw write %d", reg);

	switch (reg) {
	case BD6490FV_CLOCK_MISSING_DETECTION_PERIOD ... BD6490FV_VOLR:
		return true;
	default:
		return false;
	}
}

static const struct regmap_config bd6490fv_regmap = {
	.reg_bits = 8,
	.val_bits = 8,

	.max_register = BD6490FV_MAX_REGISTER,
	.readable_reg = bd6490fv_readable_register,
	.writeable_reg = bd6490fv_writeable_register,
	.cache_type = REGCACHE_NONE,
};

static int bd6490fv_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct bd6490fv_private *bd6490fv;
	int return_value;

	/* To tail ("wait on") the logs: */
	/* dmesg -w */
	/* (optionally use `-l info` for just info level logs) */
	pr_info("bd6490fv: Hello from driver v6!\n");

	bd6490fv = devm_kzalloc(&client->dev, sizeof(*bd6490fv), GFP_KERNEL);
	if (!bd6490fv) {
		return -ENOMEM;
	}

	i2c_set_clientdata(client, bd6490fv);

	bd6490fv->regmap = devm_regmap_init_i2c(client, &bd6490fv_regmap);
	if (IS_ERR(bd6490fv->regmap)) {
		return_value = PTR_ERR(bd6490fv->regmap);
		pr_info("bd6490fv: regmap_init() failed with: %d\n", return_value);
		return return_value;
	}

	// pr_info("bd6490fv: Probe priv %p, regmap %p\n", (void*)bd6490fv, (void*)bd6490fv->regmap);
	// regmap_write(bd6490fv->regmap, BD6490FV_CLOCK_MISSING_DETECTION_PERIOD, 6);

	return devm_snd_soc_register_component(
			&client->dev, &soc_component_dev_bd6490fv, NULL, 0);
}

static int bd6490fv_i2c_remove(struct i2c_client *client)
{
	pr_info("bd6490fv: Goodbye!\n");

	return 0;
}

static const struct of_device_id bd6490fv_of_match[] = {
	{ .compatible = "ti,bd6490fv", },
	{},
};
MODULE_DEVICE_TABLE(of, bd6490fv_of_match);

static const struct i2c_device_id bd6490fv_i2c_id[] = {
	{"bd6490fv", 0},
	{}
};
MODULE_DEVICE_TABLE(i2c, bd6490fv_i2c_id);

static struct i2c_driver bd6490fv_i2c_driver = {
	.driver = {
		.name		= "bd6490fv",
		.of_match_table	= bd6490fv_of_match,
	},
	.id_table	= bd6490fv_i2c_id,
	.probe		= bd6490fv_i2c_probe,
	.remove		= bd6490fv_i2c_remove,
};

module_i2c_driver(bd6490fv_i2c_driver);
