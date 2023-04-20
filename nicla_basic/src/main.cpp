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
uint8_t chg_reg = 0;

int main(void)
{
	printk("Hello World! %s\n", CONFIG_BOARD);
	rgbled.begin();
	pmic.enableCharge(200);
	while (1) {
		rgbled.setColor(red);
		k_msleep(5000);
		rgbled.setColor(off);
		k_msleep(2000);

		chg_reg = pmic.getChargingRegisterState();
		printk("FCHG - %d\n", chg_reg);

	}
}


