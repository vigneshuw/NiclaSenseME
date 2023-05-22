#ifndef _BOSCH_SENSORTEC_H_
#define _BOSCH_SENSORTEC_H_

#include <zephyr/types.h>
#include <zephyr/kernel.h>

#include "bosch/common/common.h"
#include "sensors/SensorTypes.h"
#ifdef __cplusplus
extern "C"
{
#endif
#include "bosch/bhy2.h"
#ifdef __cplusplus
}
#endif

#include "Buffer/CircularBuffer.h"


#define SENSOR_QUEUE_SIZE   10
#define WORK_BUFFER_SIZE    2048

#define LONG_SENSOR_QUEUE_SIZE 5
#define MAX_READ_WRITE_LEN 256


/** @brief Enumeration to check for correct short message delivery over ESLOV communication*/
enum SensorAckCode {
  SensorAck = 0x0F,    /*!< Acknowledgement */
  SensorNack = 0x00    /*!< Negative Acknowledgement */
};


class BoschSensortec {
    public:
        BoschSensortec();
        virtual ~BoschSensortec();

        /** @brief Setting up the SPI interface */
        bool begin();
        
        /** @brief Get and process the FIFOs
         * 
         * @return  true, if interrupt has occurred; false, otherwise
        */
        bool update();

        /** @brief Configure sensor properties*/
        void configureSensor(SensorConfigurationPacket& config);

        /** @brief Configure the range of the sensor*/
        int configureSensorRange(uint8_t id, uint16_t range);

        /** @brief Get the sensor configuration object for a virtual sensor */
        void getSensorConfiguration(uint8_t id, SensorConfig &virtual_sensor_conf);

        /** @brief Print Sensors to debug */
        void printSensors();

        /** @brief Check to see if the sensor corresponding to an ID is present */
        bool hasSensor(uint8_t sensorID);

        /** @brief Return available sensor data*/
        uint8_t availableSensorData();

        /** @brief Return available long sensor data */
        uint8_t availableLongSensorData();

        /** @brief Read sensor data */
        bool readSensorData(SensorDataPacket *data);

        /** @brief Read long sensor data */
        bool readLongSensorData(SensorLongDataPacket *data);

        /** @brief Handle FIFO of data queue */
        void addSensorData(SensorDataPacket *sensorData);

        /** @brief Handle FIFO of data queue for long sensor data */
        void addLongSensorData(SensorLongDataPacket *sensorData);

        /** @brief Reset NACK flag */
        uint8_t acknowledgement();


    private:
        // Circular FIFO
        CircularBufferFIFO _sensorQueue {SENSOR_QUEUE_SIZE};
        CircularBufferFIFO _sensorLongQueue {LONG_SENSOR_QUEUE_SIZE};

        uint8_t _workBuffer[WORK_BUFFER_SIZE];
        uint8_t _acknowledgement;

        struct bhy2_dev _bhy2;
        uint8_t _sensorsPresent[32];

    private:
        friend class BHY2;

};

extern BoschSensortec sensortec;


#endif