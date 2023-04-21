/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>

#include "NiclaSystem.hpp"


float battery_percentage = 0.0;

int main(void)
{
	printk("Hello World! %s\n", CONFIG_BOARD);
	nicla::leds.begin();
	nicla::pmic.enableCharge(100);
	while (1) {
		nicla::leds.setColor(red);
		k_msleep(5000);
		nicla::leds.setColor(off);
		k_msleep(2000);

		battery_percentage = nicla::pmic.getBatteryVoltage();
		printk("Battery Percentage - %f\n", battery_percentage);

	}
}


