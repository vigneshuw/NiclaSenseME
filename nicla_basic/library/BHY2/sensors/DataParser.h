#ifndef _DATA_PARSER_H_
#define _DATA_PARSER_H_

#include <zephyr/types.h>
#include "SensorTypes.h"
#include "SensorID.h"

/** @brief XYZ data*/
struct __attribute__ ((packed)) DataXYZ {
    int16_t x;
    int16_t y;
    int16_t z;
};

/** @brief Orientation data*/
struct __attribute__ ((packed)) DataOrientation {
    float heading;
    float pitch;
    float roll;
};

/** @brief Quaternion data*/
struct __attribute__ ((packed)) DataQuaternion {
    float x;
    float y;
    float z;
    float w;
    float accuracy;
};

/** @brief Gas composition data*/
struct __attribute__ ((packed)) DataBSEC {
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
        /** @brief Parse a 3D-vector data type 
         *
         * @param   data                    The sensor data packet used to store raw data
         * @param   vector                  The struct to store the data from the sensor data packet
        */
        static void parse3Dvector(SensorDataPacket &data, DataXYZ &vector);

        /** @brief Parse Euler data type 
         *
         * @param   data                    The sensor data packet used to store raw data
         * @param   vector                  The struct to store the data from the sensor data packet
        */
        static void parseEuler(SensorDataPacket &data, DataOrientation &vector);

        /** @brief Parse Euler data type 
         *
         * @param   data                    The sensor data packet used to store raw data
         * @param   vector                  The struct to store the data from the sensor data packet
         * @param   scaleFactor             Factor to scale the data components
        */
        static void parseEuler(SensorDataPacket &data, DataOrientation &vector, float scaleFactor);

        /** @brief Parse Quaternion data type 
         *
         * @param   data                    The sensor data packet used to store raw data
         * @param   vector                  The struct to store the data from the sensor data packet
         * @param   scaleFactor             Factor to scale the data components
        */
        static void parseQuaternion(SensorDataPacket &data, DataQuaternion &vector, float scaleFactor);

        /** @brief Parse BSEC data type. For the gas sensors
         *-
         * @param   data                    The sensor data packet used to store raw data
         * @param   vector                  The struct to store the data from the sensor data packet
        */
        static void parseBSEC(SensorLongDataPacket &data, DataBSEC& vector);

        /** @brief Parse BSEC data type, legacy. For the gas sensors
         *
         * @param   data                    The sensor data packet used to store raw data
         * @param   vector                  The struct to store the data from the sensor data packet
        */
        static void parseBSECLegacy(SensorLongDataPacket &data, DataBSEC &vector);

        /** @brief Parse the data payload with the specified format.
         *
         * @param   data                    The sensor data packet used to store raw data
         * @param   value                   A location to store the parsed value
         * @param   scaleFactor             Factor to scale the parsed value
         * @param   format                  Data type to extract the information. 8-bit, 16-bit, etc.,
        */
        static void parseData(SensorDataPacket &data, float &value, float scaleFactor, SensorPayload format);

        /** @brief Parse the type of activity performed
         * 
         * @param   data                    The sensor data packet used to store raw data
         * @param   value                   Location to store parsed and extracted value
        */
        static void parseActivity(SensorDataPacket &data, uint16_t &value);
};

#endif