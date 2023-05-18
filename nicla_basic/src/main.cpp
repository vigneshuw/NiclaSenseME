#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <stdio.h>
#include <string.h>
#include "NiclaSystem.hpp"

// #include "bosch/common/common.h"
#include "BoschSensortec.h"



LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);


int main(void) {

    nicla::leds.begin();
    nicla::pmic.enableCharge(100);
    bool ret = nicla::pmic.enable3V3LDO();
    if(!ret) {
        LOG_ERR("3V3LDO failed!\n");
    }
    k_msleep(5000);

    // setup_interfaces(0, BHY2_SPI_INTERFACE);
    sensortec.begin();

    while (1) {

        k_msleep(10000);

    }

}