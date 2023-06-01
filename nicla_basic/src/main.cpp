#include <zephyr/logging/log.h>

#include "NiclaSystem.hpp"
#include "BHY2.h"


LOG_MODULE_REGISTER(main, CONFIG_SET_LOG_LEVEL);


int main(void) {

    /*
    Initialize Nicla System
    */
    nicla::leds.begin();
    nicla::pmic.enableCharge(100);
    bool ret = nicla::pmic.enable3V3LDO();
    if(!ret) {
        LOG_ERR("3V3LDO failed!\n");
    }

    // Initialize
    bhy2.begin();

    while (1) {
        
        k_msleep(10000);

    }

}
