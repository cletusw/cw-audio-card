#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/kernel.h> /* Needed for pr_info() */
#include <linux/module.h>
#include <linux/regmap.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/tlv.h>

MODULE_DESCRIPTION("Implementation of a partial BD3490FV driver");
MODULE_AUTHOR("Clayton Watts <cletusw@gmail.com>");
MODULE_LICENSE("Dual MIT/GPL");

// BD3490FV actually uses multiple pages of registers. Here we just assume page 0
#define BD3490FV_INPUT_SELECTOR 0x4
#define BD3490FV_VOLL 0x21
#define BD3490FV_VOLR 0x22
#define BD3490FV_RESET 0xFE

#define BD3490FV_MAX_REGISTER BD3490FV_RESET
#define BD3490FV_INPUT_SELECTOR_AUX_VALUE 0x00
#define BD3490FV_INPUT_SELECTOR_DAC_VALUE 0x01
// TODO: Figure out how to handle mute value, 0xFF
#define BD3490FV_MAX_VOLUME_VALUE 0xD7	// Inverted, so this is actually lowest volume
#define BD3490FV_MIN_VOLUME_VALUE 0x80	// Inverted, so this is actually highest volume
#define BD3490FV_RESET_VALUE 0x81

/* Private data for the BD3490FV */
struct bd3490fv_private {
	struct regmap *regmap;
};

static const struct reg_default bd3490fv_reg_defaults[] = {
	{ BD3490FV_INPUT_SELECTOR, 0x07 },
	{ BD3490FV_VOLL, BD3490FV_MAX_VOLUME_VALUE },
	{ BD3490FV_VOLR, BD3490FV_MAX_VOLUME_VALUE },
	{ BD3490FV_RESET, 0x00 },
};

static const DECLARE_TLV_DB_SCALE(volume_tlv, -8700, 100, 0 /* mute = false */);

static const struct snd_kcontrol_new bd3490fv_snd_controls[] = {
	SOC_DOUBLE_R_RANGE_TLV(
			"Hardware Master Playback Volume",
			BD3490FV_VOLL,
			BD3490FV_VOLR,
			/* xshift = */ 0,
			BD3490FV_MIN_VOLUME_VALUE,
			BD3490FV_MAX_VOLUME_VALUE,
			/* invert = */ 1,
			volume_tlv),
};

// Input selector
static const char * const bd3490fv_input_selector_text[] = {
	"Pi",
	"Aux",
};
static const struct soc_enum bd3490fv_input_selector_enum =
	SOC_ENUM_SINGLE(
			BD3490FV_INPUT_SELECTOR,
			/* xshift = */ 0,
			ARRAY_SIZE(bd3490fv_input_selector_text),
			bd3490fv_input_selector_text);
static const struct snd_kcontrol_new bd3490fv_input_selector =
	SOC_DAPM_ENUM("SOURCEMUX", bd3490fv_input_selector_enum);

static int bd3490fv_dapm_event(struct snd_soc_dapm_widget *w, struct snd_kcontrol *k, int event) {
	// pr_info("bd3490fv: pre DAPM event: %d", event);
	struct bd3490fv_private *bd3490fv = snd_soc_component_get_drvdata(w->dapm->component);
	int ret = 0;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		pr_info("bd3490fv: Change mux to DAC");
		// pr_info("bd3490fv: To DAC priv %p, regmap %p\n", (void*)bd3490fv, (void*)bd3490fv->regmap);
		ret = regmap_write(bd3490fv->regmap, BD3490FV_INPUT_SELECTOR, BD3490FV_INPUT_SELECTOR_DAC_VALUE);
		break;
	case SND_SOC_DAPM_PRE_PMD:
		pr_info("bd3490fv: Change mux to AUX");
		// pr_info("bd3490fv: To AUX priv %p, regmap %p\n", (void*)bd3490fv, (void*)bd3490fv->regmap);
		ret = regmap_write(bd3490fv->regmap, BD3490FV_INPUT_SELECTOR, BD3490FV_INPUT_SELECTOR_AUX_VALUE);
		break;
	}
	if (ret < 0) {
		pr_info("bd3490fv: Error changing mux: %d", ret);
		return -EIO;
	}
	return 0;
}

static const struct snd_soc_dapm_widget bd3490fv_dapm_widgets[] = {
	SND_SOC_DAPM_INPUT("DACINL"),
	SND_SOC_DAPM_INPUT("DACINR"),
	SND_SOC_DAPM_INPUT("AUXINL"),
	SND_SOC_DAPM_INPUT("AUXINR"),

	SND_SOC_DAPM_PRE("DAC DAPM listener", bd3490fv_dapm_event),
	SND_SOC_DAPM_MUX("Source select", SND_SOC_NOPM, 0, 0, &bd3490fv_input_selector),

	SND_SOC_DAPM_OUTPUT("MUXOUTL"),
	SND_SOC_DAPM_OUTPUT("MUXOUTR"),
};

static const struct snd_soc_dapm_route bd3490fv_routes[] = {
	{ "DAC DAPM listener", NULL, "DACINL" },
	{ "DAC DAPM listener", NULL, "DACINR" },
	{ "Source select", NULL, "DAC DAPM listener" },
	{ "Source select", NULL, "AUXINL" },
	{ "Source select", NULL, "AUXINR" },

	{ "MUXOUTL", NULL, "Source select" },
	{ "MUXOUTR", NULL, "Source select" },
};

static const struct snd_soc_component_driver soc_component_dev_bd3490fv = {
	.controls = bd3490fv_snd_controls,
	.num_controls = ARRAY_SIZE(bd3490fv_snd_controls),
	.dapm_widgets = bd3490fv_dapm_widgets,
	.num_dapm_widgets = ARRAY_SIZE(bd3490fv_dapm_widgets),
	.dapm_routes = bd3490fv_routes,
	.num_dapm_routes = ARRAY_SIZE(bd3490fv_routes),
	.idle_bias_on = 1,
	.use_pmdown_time= 1,
	.endianness = 1,
	.non_legacy_dai_naming = 1,
};

static bool bd3490fv_readable_register(struct device *dev, unsigned int reg)
{
	pr_info("bd3490fv: hw read %d", reg);

	// BD3490 doesn't allow i2c reads
	return false;
}

static bool bd3490fv_writeable_register(struct device *dev, unsigned int reg)
{
	pr_info("bd3490fv: hw write %d", reg);

	switch (reg) {
	case BD3490FV_INPUT_SELECTOR ... BD3490FV_RESET:
		return true;
	default:
		return false;
	}
}

static const struct regmap_config bd3490fv_regmap = {
	.reg_bits = 8,
	.val_bits = 8,

	.max_register = BD3490FV_MAX_REGISTER,
	.readable_reg = bd3490fv_readable_register,
	.writeable_reg = bd3490fv_writeable_register,
	.reg_defaults = bd3490fv_reg_defaults,
	.num_reg_defaults = ARRAY_SIZE(bd3490fv_reg_defaults),
	.cache_type = REGCACHE_RBTREE,
};

static int bd3490fv_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct bd3490fv_private *bd3490fv;
	int return_value;

	/* To tail ("wait on") the logs: */
	/* dmesg -w */
	/* (optionally use `-l info` for just info level logs) */
	pr_info("bd3490fv: Hello from driver v6!\n");

	bd3490fv = devm_kzalloc(&client->dev, sizeof(*bd3490fv), GFP_KERNEL);
	if (!bd3490fv) {
		return -ENOMEM;
	}

	i2c_set_clientdata(client, bd3490fv);

	bd3490fv->regmap = devm_regmap_init_i2c(client, &bd3490fv_regmap);
	if (IS_ERR(bd3490fv->regmap)) {
		return_value = PTR_ERR(bd3490fv->regmap);
		pr_info("bd3490fv: regmap_init() failed with: %d\n", return_value);
		return return_value;
	}

	return_value = regmap_write(bd3490fv->regmap, BD3490FV_RESET, BD3490FV_RESET_VALUE);
	if (return_value < 0) {
		pr_info("Error performing intial reset: %d\n", return_value);
		return -EIO;
	}

	return_value = regmap_write(bd3490fv->regmap, BD3490FV_INPUT_SELECTOR, BD3490FV_INPUT_SELECTOR_AUX_VALUE);
	if (return_value < 0) {
		pr_info("Error initializing input to AUX: %d\n", return_value);
		return -EIO;
	}

	// pr_info("bd3490fv: Probe priv %p, regmap %p\n", (void*)bd3490fv, (void*)bd3490fv->regmap);
	// regmap_write(bd3490fv->regmap, BD3490FV_INPUT_SELECTOR, 6);

	return devm_snd_soc_register_component(
			&client->dev, &soc_component_dev_bd3490fv, NULL, 0);
}

static int bd3490fv_i2c_remove(struct i2c_client *client)
{
	pr_info("bd3490fv: Goodbye!\n");

	return 0;
}

static const struct of_device_id bd3490fv_of_match[] = {
	{ .compatible = "ti,bd3490fv", },
	{},
};
MODULE_DEVICE_TABLE(of, bd3490fv_of_match);

static const struct i2c_device_id bd3490fv_i2c_id[] = {
	{"bd3490fv", 0},
	{}
};
MODULE_DEVICE_TABLE(i2c, bd3490fv_i2c_id);

static struct i2c_driver bd3490fv_i2c_driver = {
	.driver = {
		.name		= "bd3490fv",
		.of_match_table	= bd3490fv_of_match,
	},
	.id_table	= bd3490fv_i2c_id,
	.probe		= bd3490fv_i2c_probe,
	.remove		= bd3490fv_i2c_remove,
};

module_i2c_driver(bd3490fv_i2c_driver);
