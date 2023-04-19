/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gap.h>

#include "RGBled.hpp"

RGBled rgbled;

int main(void)
{
	printk("Hello World! %s\n", CONFIG_BOARD);
	rgbled.begin();
	while (1) {
		rgbled.setColor(magenta);
		k_msleep(5000);
		rgbled.setColor(off);
		k_msleep(2000);
	}
}


