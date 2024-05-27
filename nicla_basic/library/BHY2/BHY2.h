#ifndef _BHY2_H_
#define _BHY2_H_

#include <zephyr/types.h>
#include "sensors/SensorTypes.h"
#include "sensors/DataParser.h"

#include "sensors/SensorID.h"
#include "sensors/SensorClass.h"
#include "sensors/SensorOrientation.h"
#include "sensors/SensorXYZ.h"
#include "sensors/SensorQuaternion.h"
#include "sensors/SensorBSEC.h"
#include "sensors/SensorActivity.h"
#include "sensors/Sensor.h"


class BHY2 {
    public:
        BHY2();
        virtual ~BHY2();

        /** @brief Initialize Nicla System and the BHI260 sensor system
         * 
         * @return true, if both the system initialization were successful; 0, otherwise
        */
        bool begin();

        /** @brief Look for data in FIFO, parse and call all the registered callbacks
         * 
         * @retval  true    If the interrupt has occurred and data is present
         * @retval  false   Otherwise
        */
        bool update();

        /** @brief  Look for data in FIFO, parse and call all the registered callbacks. Additionally, 
         *          delay the thread for a set amount of time
         * 
         * @param   ms                  Time in milliseconds
         * 
         * @retval  true    If the interrupt has occurred and data is present
         * @retval  false   Otherwise
        */
        bool update(unsigned long ms);

        /** @brief Delay for a specified amount of ms 
         * 
         * @param   ms                  Time in milliseconds
         * 
        */
        void delay(unsigned long ms);

        /** @brief Configure the sensor 
         * 
         * @param   config              Sensor configuration packet; Virtual Sensor
         * 
        */
        void configureSensor(SensorConfigurationPacket &config);
        
        /** @brief Configure the sensor
         * 
         * @param   sensorID            ID of the sensor
         * @param   sampleRate          The sampling rate of the sensor
         * @param   latency             Time to wait before an interrupt was generated
        */
        void configureSensor(uint8_t sensorID, float sampleRate, uint32_t latency);

        /** @brief Handle the FIFO data queue
         * 
         * @param   sensorData          Data packet from sensor
        */
        void addSensorData(SensorDataPacket &sensorData);

        /** @brief Handle FIFO data queue for long sensor data
         * 
         * @param   sensorData          Data paclet from long sensor 
        */
        void addLongSensorData(SensorLongDataPacket &sensorData);

        /** @brief Return available sensor data
         * 
         * @return  uint8_t
        */
        uint8_t availableSensorData();

        /** @brief Return available sensor data
         * 
         * @return  uint8_t
        */
        uint8_t availableLongSensorData();

        /** @brief Read the sensor data
         * 
         * @param   data                Data packet to read into for the sensor
         * 
         * @return  Status of the read
        */
        bool readSensorData(SensorDataPacket *data);

        /** @brief Read long sensor data
         * 
         * @param   data                Data packet to read into for the sensor
         * 
         * @return  Status of the read
        */
        bool readLongSensorData(SensorLongDataPacket *data);

        /** @brief Check if a sensor with the sensorID is available
         * 
         * @param   sensorID            The ID for the sensor
         * 
         * @return  true, if sensor is available; false it is not
        */
        bool hasSensor(uint8_t sensorID);

        /** @brief Parse the XYZ Cartesian data
         * 
         * @param   data                Data packet containing the sensor ID
         * @param   vector              Vector with XYZ to parse the data into
        */
        void parse(SensorDataPacket &data, DataXYZ &vector);

        /** @brief Parse the orientation data
         * 
         * @param   data                Data packet with the sensor ID
         * @param   vector              Vector with heading, pitch, and roll
        */
        void parse(SensorDataPacket &data, DataOrientation &vector);

        /** @brief Parse the orientation data
         * 
         * @param   data                Data packet with the sensor ID
         * @param   vector              Vector with heading, pitch, and roll
         * @param   scaleFactor         scale factor for vector
        */
        void parse(SensorDataPacket &data, DataOrientation &vector, float scaleFactor);

        /** @brief Define the LDO regulator timeout 
         * 
         * @param   time                In milliseconds; Default is 120000ms
        */
        void setLDOTimeout(int time);


    private:
        int64_t _pingTime;
        int _timeout;
        int64_t _startTime;

};


/** @brief The BHY2 class can be externally linked to bhy2 in the main.cpp*/
extern BHY2 bhy2;

#endif