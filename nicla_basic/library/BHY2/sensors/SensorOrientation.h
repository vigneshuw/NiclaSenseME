#ifndef _SENSOR_ORIENTATION_H_
#define _SENSOR_ORIENTATION_H_

#include "SensorClass.h"


class SensorOrientation : public SensorClass {
    public:
        SensorOrientation() {}

        SensorOrientation(uint8_t id) : SensorClass(id), _data(), _factor(0) {
            for (int i = 0; i < NUM_SUPPORTED_SENSOR; i++) {
                if(SensorList[i].id == id) {
                    _factor = SensorList[i].scaleFactor;
                }
            }
        }

        /** @brief Get the heading */
        float heading() {
            return _data.heading;
        }
        
        /** @brief Get the pitch */
        float pitch() {
            return _data.pitch;
        }

        /** @brief Get the roll */
        float roll() {
            return _data.roll;
        }

        /** @brief Set the scale factor
         * @param   factor                  The scaling factor value
        */
        void setFactor(float factor) {
            _factor = factor;
        }

        /** @brief Get the scale factor 
         * 
         * @retval  The scaling factor
        */
        float getFactor() {
            return _factor;
        }

        /** @brief Set the data to the SensorClass 
         * 
         * @param   data                    Pointer to the sensor data packet
        */
        void setData(SensorDataPacket &data) {
            DataParser::parseEuler(data, _data, _factor);
        }

        /** @brief Set the data (Long data) to SensorClass. */
        void setData(SensorLongDataPacket &data) {}


    private:
        DataOrientation _data;
        float _factor;

};


#endif