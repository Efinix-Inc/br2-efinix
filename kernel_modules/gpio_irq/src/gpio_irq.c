/*
 * Copyright (C) 2025 Efinix Inc. All rights reserved.
 *
 * SPDX-License-Identifier: GPL-3.0
 *
 */
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/kernel.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>
#include <linux/irq.h>

struct gpio_irq_dev {
	struct gpio_desc *gpio;
	struct device *dev;
	int irq;
};

static irqreturn_t gpio_irq_handler(int irq, void *dev_id)
{
	struct gpio_irq_dev *gd = (struct gpio_irq_dev *)dev_id;

	dev_info(gd->dev, "GPIO interrupt triggered! IRQ=%d\n", gd->irq);
	return IRQ_HANDLED;
}

static int gpio_irq_probe(struct platform_device *pdev)
{
	struct gpio_irq_dev *data;
	int ret;

	dev_info(&pdev->dev, "Probing GPIO IRQ driver...\n");

	data = devm_kzalloc(&pdev->dev, sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;
	
	data->dev = &pdev->dev;
	data->irq = platform_get_irq(pdev, 0);
	if (data->irq < 0)
		return data->irq;

	dev_info(&pdev->dev, "Get IRQ=%d\n", data->irq);
	ret = devm_request_irq(&pdev->dev, data->irq, gpio_irq_handler,
			       IRQF_TRIGGER_NONE, "gpio_irq", data);

	if (ret) {
		dev_err(&pdev->dev, "Failed to request IRQ %d\n", data->irq);
		return ret;
	}

	platform_set_drvdata(pdev, data);
	dev_info(&pdev->dev, "GPIO IRQ driver loaded successfully\n");
	
	return 0;
}

static int gpio_irq_remove(struct platform_device *pdev)
{
	dev_info(&pdev->dev, "Removing GPIO IRQ driver...\n");
	return 0;
}

static const struct of_device_id gpio_irq_of_match[] = {
	{ .compatible = "efinix,gpio-irq" },
	{ },
};
MODULE_DEVICE_TABLE(of, gpio_irq_of_match);

static struct platform_driver gpio_irq_driver = {
	.probe = gpio_irq_probe,
	.remove = gpio_irq_remove,
	.driver = {
		.name = "gpio_irq",
		.of_match_table = gpio_irq_of_match,
	},
};

module_platform_driver(gpio_irq_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mohamd Noor Alim <mnalim@efinixinc.com");
MODULE_DESCRIPTION("Simple GPIO interrupt kernel module");
