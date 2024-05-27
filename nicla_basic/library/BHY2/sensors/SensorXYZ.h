#ifndef _SENSOR_XYZ_H_
#define _SENSOR_XYX_H_

#include "SensorClass.h"


class SensorXYZ: public SensorClass {
    public:
        SensorXYZ() {}

        SensorXYZ(uint8_t id) : SensorClass(id), _data() {}

        int16_t x() {
            return _data.x;
        }

        int16_t y() {
            return _data.y;
        }

        int16_t z() {
            return _data.z;
        }

        DataXYZ getData() {
            return _data;
        }

        void setData(SensorDataPacket &data) {
            DataParser::parse3Dvector(data, _data);
        }

        void setData(SensorLongDataPacket &data) {}

    
    private:
        DataXYZ _data;

};


#endif