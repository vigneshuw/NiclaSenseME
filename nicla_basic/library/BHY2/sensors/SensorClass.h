#ifndef _SENSOR_CLASS_H_
#define _SENSOR_CLASS_H_

#include <zephyr/types.h>

#include "SensorID.h"
#include "DataParser.h"
#include "BoschSensortec.h"


class SensorClass {
    public:
        __attribute__ ((error("Sensor requires an ID"))) SensorClass();
        SensorClass(uint8_t id);
        virtual ~SensorClass();

        /** @brief Function to get the Sensor ID corresponding to this object
         * 
         * @return The sensor ID
        */
        uint8_t id();

        /** @brief Configure the virtual sensor at BHI260AP
         * 
         * @param rate          The sampling rate
         * @param latency       BHI260 FIFO data report latency
         * 
         * @return 1 - Sensor Present and Configured successfully; 0 - Otherwise
        */
        bool begin(float rate = 1000, uint32_t latency = 1);

        void configure(float rate, uint32_t latency);

        /** @brief Set the range for the sensor 
         * 
         * @param range         The range to set for the sensor
         * 
         * @result Status of the configuration
         * */        
        int setRange(uint16_t range);
        const SensorConfig getConfiguration();
        void end();

        virtual void setData(SensorDataPacket &data) = 0;
        virtual void setData(SensorLongDataPacket &data) = 0;
        // virtual char *toString() = 0;

    protected:
        uint8_t _id;
        bool _subscribed;

};

#endif