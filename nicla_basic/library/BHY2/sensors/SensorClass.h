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
         * @retval  The sensor ID
        */
        uint8_t id();

        /** @brief Begin and Configure the virtual sensor at BHI260AP
         * 
         * @param   rate                    The sampling rate
         * @param   latency                 BHI260 FIFO data report latency
         * 
         * @retval  1, if Sensor Present and Configured successfully
         * @retval  0, Otherwise
        */
        bool begin(float rate = 1000, uint32_t latency = 1);

        /** @brief Configure the initialized virtual sensor
         * 
         * @param   rate                    The sampling rate
         * @param   latency                 BHI260 FIFO data report latency
         * 
        */
        void configure(float rate, uint32_t latency);

        /** @brief Set the range for the sensor 
         * 
         * @param   range                   The range to set for the sensor
         * 
         * @retval  Status of the configuration
         * */        
        int setRange(uint16_t range);

        /** @brief Get the configuration of the sensor, belonging to the a SensorClass
         * 
         * @retval  SensorConfig
         * 
        */
        const SensorConfig getConfiguration();

        /** @brief End the sensors*/
        void end();

        /** @brief Set data to the SensorClass 
         * 
         * @param   data                    The data packet for the sensor                
        */
        virtual void setData(SensorDataPacket &data) = 0;

        /** @brief Set data to the SensorClass. SensorLongDataPacket. 
         * 
         * @param   data                    The data packet for the sensor
        */
        virtual void setData(SensorLongDataPacket &data) = 0;
        // virtual char *toString() = 0;

    protected:
        uint8_t _id;
        bool _subscribed;

};

#endif