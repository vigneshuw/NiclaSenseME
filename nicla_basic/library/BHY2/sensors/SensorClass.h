#ifndef _SENSOR_CLASS_H_
#define _SENSOR_CLASS_H_

#include <zephyr/types.h>

#include "SensorID.h"
#include "DataParser.h"


class SensorClass {
    public:
        __attribute__ ((error("Sensor requires an ID"))) SensorClass();
        SensorClass(uint8_t id);
        virtual ~SensorClass();

        uint8_t id();

        bool begin(float rate = 1000, uint32_t latency = 1);
        void configure(float rate, uint32_t latency);
        int setRange(uint16_t range);
        const SensorConfig getConfiguration();
        void end();

        virtual void setData(SensorDataPacket &data) = 0;
        virtual void setData(SensorLongDataPacket &data) = 0;
        // virtual String toString() = 0;

    protected:
        uint8_t _id;
        bool _subscribed;

};

#endif