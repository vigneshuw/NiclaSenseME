#ifndef _SENSOR_QUATERNION_H_
#define _SENSOR_QUATERNION_H_

#include "SensorClass.h"


class SensorQuaternion: public SensorClass {
    public:
        SensorQuaternion() {}

        SensorQuaternion(uint8_t id): SensorClass(id), _data(), _factor(0.000061035) {}

        float x() { 
            return _data.x; 
        }

        float y() {
            return _data.y;
        }

        float z() {
            return _data.z;
        }

        float w() {
            return _data.w;
        }

        float accuracy() {
            return _data.accuracy;
        }

        float getFactor() {
            return _factor;
        }

        /** @brief Set the data to the SensorClass 
         * 
         * @param   data                    Pointer to the sensor data packet
        */
        void setData(SensorDataPacket &data) {
            DataParser::parseQuaternion(data, _data, _factor);
        }

        /** @brief Set the data (Long data) to SensorClass. */
        void setData(SensorLongDataPacket &data) {}


    private:
        DataQuaternion _data;
        float _factor;
};


#endif