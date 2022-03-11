#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/mod_devicetable.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/gpio/consumer.h>

#undef pr_fmt
#define pr_fmt(fmt) "%s : " fmt, __func__

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ALIM");
MODULE_DESCRIPTION("A gpio consumer driver");

/* Device private data structure */
struct gpiodev_private_data {
	char label[20];
	struct gpio_desc *desc;
};

/* Driver private data structure */
struct gpiodrv_private_data {
	int total_devices;
	struct class *class_gpio;
	struct device **dev;
};

struct gpiodrv_private_data gpio_drv_data;

ssize_t direction_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int dir;
	char *direction;

	struct gpiodev_private_data *dev_data = dev_get_drvdata(dev);

	dir = gpiod_get_direction(dev_data->desc);
	if (dir < 0)
		return dir;

	/* if dir = 0, then show "out". If dir = 1, then show "in" */
	direction = (dir == 0) ? "out": "in";

	return sprintf(buf, "%s\n", direction);
}

ssize_t direction_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	struct gpiodev_private_data *dev_data = dev_get_drvdata(dev);

	if (sysfs_streq(buf, "in"))
		ret = gpiod_direction_input(dev_data->desc);
	else if (sysfs_streq(buf, "out"))
		ret = gpiod_direction_output(dev_data->desc, 0);
	else
		ret = -EINVAL;

	return ret ? ret: count;
}

ssize_t value_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int value;
	struct gpiodev_private_data *dev_data = dev_get_drvdata(dev);
	value = gpiod_get_value(dev_data->desc);
        return sprintf(buf, "%d\n", value);
}

ssize_t value_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	long value;
	struct gpiodev_private_data *dev_data = dev_get_drvdata(dev);

	ret = kstrtol(buf, 0, &value);
	if (ret)
		return ret;
	gpiod_set_value(dev_data->desc, value);
        return count; 
}

ssize_t label_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct gpiodev_private_data *dev_data = dev_get_drvdata(dev);
        return sprintf(buf, "%s\n", dev_data->label);
}

/* create device attribute sysfs */
static DEVICE_ATTR_RW(direction);
static DEVICE_ATTR_RW(value);
static DEVICE_ATTR_RO(label);

/* list of individual attribute */
static struct attribute *gpio_attrs[] = {
	&dev_attr_direction.attr,
	&dev_attr_value.attr,
	&dev_attr_label.attr,
	NULL
};

/* group of attribute */
static struct attribute_group gpio_attr_group = {
	.attrs = gpio_attrs
};


/* create pointer array pointer group. List of attribute group */
static const struct attribute_group *gpio_attr_groups[] = {
	&gpio_attr_group,
	NULL
};

int gpio_sysfs_remove(struct platform_device *pdev)
{
	int i;
	dev_info(&pdev->dev, "Remove device\n");
	for (i = 0; i < gpio_drv_data.total_devices; i++) {
		device_unregister(gpio_drv_data.dev[i]);
	}
	return 0;
}

int gpio_sysfs_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct gpiodev_private_data *dev_data;
	const char *name;
	int i = 0;
	int ret;

	/* driver need to read dts file and iterate the child node of gpio. */

	/* parent device node */
	struct device_node *parent = pdev->dev.of_node;
	struct device_node *child = NULL;

	gpio_drv_data.total_devices = of_get_child_count(parent);
	if (!gpio_drv_data.total_devices) {
		dev_err(dev, "No devices found\n");
		return -EINVAL;
	}

	dev_info(dev, "Total devices found  = %d\n", gpio_drv_data.total_devices);
	gpio_drv_data.dev = devm_kzalloc(dev, sizeof(struct device *) * gpio_drv_data.total_devices, GFP_KERNEL);

	for_each_available_child_of_node(parent, child)
	{
		/* allocate memory for device private data */
		dev_data = devm_kzalloc(dev, sizeof(*dev_data), GFP_KERNEL);
		if (!dev_data)
		{
			dev_err(dev, "Cannot allocate memory\n");
			return -ENOMEM;
		}

		if (of_property_read_string(child, "label", &name))
		{
			dev_warn(dev, "Missing label information\n");
			snprintf(dev_data->label, sizeof(dev_data->label), "unkngpio%d", i);
		} else {
			strcpy(dev_data->label, name);
			dev_info(dev, "GPIO label = %s\n", dev_data->label);
		}

		/* we dont have struct device *dev from the child node. Thus, we cannot use gpiod_get().
		 * To obtain gpio from a child, we need to use devm_fwnode_get_gpiod_from_child(). */
		dev_data->desc = devm_fwnode_get_gpiod_from_child(dev, "efx", &child->fwnode, GPIOD_ASIS, dev_data->label);

		if (IS_ERR(dev_data->desc)) {
			ret = PTR_ERR(dev_data->desc);
			if (ret == -ENOENT)
				dev_err(dev, "No GPIO has been assigned to the requested function and/or index\n");
			return ret;
		}

		/* set the gpio direction to output */
		ret = gpiod_direction_output(dev_data->desc, 0);
		if (ret) {
			dev_err(dev, "gpio direction set failed\n");
			return ret;
		}

		/* create devices under /sys/class/efx_gpios */
		gpio_drv_data.dev[i] = device_create_with_groups(gpio_drv_data.class_gpio, dev, 0, dev_data, gpio_attr_groups, dev_data->label);
		if (IS_ERR(gpio_drv_data.dev[i])) {
			dev_err(dev, "Error in device_create\n");
			return PTR_ERR(gpio_drv_data.dev[i]);
		}


		i++;
	}
	return 0;
}

struct of_device_id gpio_device_match[] = {
	{.compatible = "efinix,efx-gpio"},
	{ }
};

struct platform_driver gpiosysfs_platform_driver = {
	.probe = gpio_sysfs_probe,
	.remove = gpio_sysfs_remove,
	.driver = {
		.name = "efx-gpio",
		.of_match_table = of_match_ptr(gpio_device_match)
	}
};

int __init gpio_sysfs_init(void)
{
	gpio_drv_data.class_gpio = class_create(THIS_MODULE, "efx_gpios");
	if (IS_ERR(gpio_drv_data.class_gpio))
	{
		pr_err("Error in creating class \n");
		return PTR_ERR(gpio_drv_data.class_gpio);
	}

	platform_driver_register(&gpiosysfs_platform_driver);

	pr_info("Module load success\n");
	return 0;
}

void __exit gpio_sysfs_exit(void)
{
	platform_driver_unregister(&gpiosysfs_platform_driver);
	class_destroy(gpio_drv_data.class_gpio);
	pr_info("Module unloaded\n");
}

module_init(gpio_sysfs_init);
module_exit(gpio_sysfs_exit);
