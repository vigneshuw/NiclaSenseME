#ifndef _DATA_PARSER_H_
#define _DATA_PARSER_H_

#include <zephyr/types.h>
#include "SensorTypes.h"
#include "SensorID.h"


struct DataXYZ {
    int16_t x;
    int16_t y;
    int16_t z;
};

struct DataOrientation {
    float heading;
    float pitch;
    float roll;
};

struct DataQuaternion {
    float x;
    float y;
    float z;
    float w;
    float accuracy;
};

struct DataBSEC {
   uint16_t  iaq;          //iaq value for regular use case
    uint16_t  iaq_s;       //iaq value for stationary use cases
    float     b_voc_eq;    //breath VOC equivalent (ppm)
    uint32_t  co2_eq;      //CO2 equivalent (ppm) [400,]
    float     comp_t;      //compensated temperature (celcius)
    float     comp_h;      //compensated humidity
    uint32_t  comp_g;      //compensated gas resistance (Ohms)
    uint8_t   accuracy;    //accuracy level: [0-3]

};


class DataParser {
    public: 
        static void parse3Dvector(SensorDataPacket &data, DataXYZ &vector);
        static void parseEuler(SensorDataPacket &data, DataOrientation &vector);
        static void parseEuler(SensorDataPacket &data, DataOrientation &vector, float scaleFactor);
        static void parseQuaternion(SensorDataPacket &data, DataQuaternion &vector, float scaleFactor);
        static void parseBSEC(SensorLongDataPacket &data, DataBSEC& vector);
        static void parseBSECLegacy(SensorLongDataPacket &data, DataBSEC &vector);
        static void parseData(SensorDataPacket &data, float &value, float scaleFactor, SensorPayload format);
        static void parseActivity(SensorDataPacket &data, uint16_t &value);
};

#endif