/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gap.h>

#include "RGBled.hpp"
#include "BQ25120A.hpp"

RGBled rgbled;
BQ25120A pmic;
float battery_percentage = 0.0;

int main(void)
{
	printk("Hello World! %s\n", CONFIG_BOARD);
	rgbled.begin();
	pmic.enableCharge(100);
	while (1) {
		rgbled.setColor(red);
		k_msleep(5000);
		rgbled.setColor(off);
		k_msleep(2000);

		battery_percentage = pmic.getBatteryVoltage();
		printk("Battery Percentage - %f\n", battery_percentage);

	}
}


