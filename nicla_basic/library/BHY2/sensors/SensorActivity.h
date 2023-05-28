#ifndef _SENSOR_ACTIVITY_H_
#define _SENSOR_ACTIVITY_H_

#include "SensorClass.h"


/** @brief SensorActivity class extends the SensorClass */
class SensorActivity: public SensorClass {
    public:
        SensorActivity() {}

        /** @brief Initialize Sensor Activity. Extends the SensorClass
         * 
         * @param   id                  The ID to initialize the sensor. Depends on the BHI260AP
        */
        SensorActivity(uint8_t id) : SensorClass(id), _value(0) {}

        /** @brief Get the value for the activity 
         * 
         * @retval  The value for the activity
        */
        uint16_t value() {
            return _value;
        }

        /** @brief Set the data to the Sensor Class
         * 
         * @param   data                    Pointer to the SensorDataPacket
        */
        void setData(SensorDataPacket &data) {
            DataParser::parseActivity(data, _value);
        }

        /** @brief Set the data to SensorLongDataPacket */
        void setData(SensorLongDataPacket &data) {}


    private:
        uint16_t _value;
        // ActivityBitMask _activityArray[16] = {
        //     {0,  "Still activity ended"},
        //     {1,  "Walking activity ended"},
        //     {2,  "Running activity ended"},
        //     {3,  "On bicycle activity ended"},
        //     {4,  "In vehicle activity ended"},
        //     {5,  "Tilting activity ended"},
        //     {6,  "In vehicle still ended"},
        //     {7,  ""},
        //     {8,  "Still activity started"},
        //     {9,  "Walking activity started"},
        //     {10, "Running activity started"},
        //     {11, "On bicycle activity started"},
        //     {12, "IN vehicle activity started"},
        //     {13, "Tilting activity started"},
        //     {14, "In vehicle still started"},
        //     {15, ""}
        // };
};

#endif