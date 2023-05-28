#ifndef _SENSOR_H_
#define _SENSOR_H_

#include "SensorClass.h"


class Sensor: public SensorClass {
    public:
        Sensor() {}

        /** @brief Initialization of the Sensor class 
         * 
         * @param   id                  The ID for the sensor. The ID depends on the ID specified in BHI20AP firmware
        */
        Sensor(uint8_t id) : SensorClass(id), _value(0.), _factor(0) {
            for(int i = 0; i < NUM_SUPPORTED_SENSOR; i++) {
                if(SensorList[i].id == id) {
                    _factor = SensorList[i].scaleFactor;
                    _format = SensorList[i].payload;
                }
            }
        }

        /** @brief Get the value. 
         * 
         * @retval  1, if there is an activity detected
         * @retval  0, if 
        */
        float value() {
            if(_format == PEVENT) {
                if(_value > 0) {
                    _value = 0;
                    return 1;
                }
                return 0;
            }
            return _value;
        }

        /** @brief Set the scaling factor for sensor output
         * 
         * @param   factor                  The scaling factor
        */
        void setFactor(float factor) {
            _factor = factor;
        }

        /** @brief Get the currently set scaling factor
         * 
         * @retval  The scaling factor
        */
        float getFactor() {
            return _factor;
        }

        /** @brief Set the data to the Sensor class 
         * 
         * @param   data                    Pointer to the SensorDataPacket
        */
        void setData(SensorDataPacket &data) {
            DataParser::parseData(data, _value, _factor, _format);
        }

        /** @brief Set the data to the Sensor class, if it is a SensorLongDataPacket. 
         * 
         * @note    Empty function.
        */
        void setData(SensorLongDataPacket &data) {}


    private:
        float _value;
        float _factor;
        SensorPayload _format;

};


#endif
