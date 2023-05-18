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

        float heading() {
            return _data.heading;
        }

        float pitch() {
            return _data.pitch;
        }

        float roll() {
            return _data.roll;
        }

        void setFactor(float factor) {
            _factor = factor;
        }

        void getFactor() {
            return _factor;
        }

        void setData(SensorDataPacket &data) {
            DataParser::parseEuler(data, _data, _factor);
        }

        void setData(SensorLongDataPacket &data) {}


    private:
        DataOrientation _data;
        float _factor;

};


#endif