#include <zephyr/logging/log.h>

#include "NiclaSystem.hpp"
#include "BHY2.h"


LOG_MODULE_REGISTER(main, CONFIG_SET_LOG_LEVEL);


// Acceleration sensor
SensorXYZ accel(SENSOR_ID_ACC_PASS);


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
    accel.begin(400.0, 0);

    while (1) {
        
        bhy2.update(1000);
        printk("X-%d, Y-%d, Z-%d\n", accel.x(), accel.y(), accel.z());

    }

}
