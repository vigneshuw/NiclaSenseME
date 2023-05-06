#ifndef _NICLASYSTEM_H_
#define _NICLASYSTEM_H_

#include <zephyr/kernel.h>
#include "RGB/RGBled.hpp"
#include "PMIC/BQ25120A.hpp"
#include "SPIFLASH/MX25R1635F.hpp"


#define BATTERY_COLD        (1 << 4)
#define BATTERY_COOL        (2 << 4)
#define BATTERY_HOT         (3 << 4)
#define BATTERY_CHARGING    (1 << 7)

class nicla{

    public:
        // I2C devices
        static RGBled leds;
        static BQ25120A pmic;

        // SPI devices
        static MX25R1635F spiFLash;

};




#endif