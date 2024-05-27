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

        /** @brief Get and return boot status*/
        uint8_t get_boot_status();

        /** @brief Erase BHY2 Flash using the provided address range
         * 
         * @param   start_addr  The starting address for the flash erase
         * @param   end_addr    The ending address for the flash erase
         * 
         * @retval  BHY2 API Status
        */
        int8_t erase_bhy2_flash(uint32_t start_addr, uint32_t end_addr);

        /** @brief Update interrupt control register 
         * 
         * @param   hintr_ctrl  The value to update the host interrupt control register
        */
        int8_t update_host_interrupt_ctrl(uint8_t hintr_ctrl);

        /** @brief Update the host interface control
         * 
         * @param   hit_ctrl    The value to update the host interface control register
        */
        int8_t update_host_interface_ctrl(uint8_t hif_ctrl);

        /** @brief Soft reset the BHY2 device */
        int8_t soft_reset_bhy2_device();
        
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

        /** @brief Print if any errors in the API result */
        void print_api_error(int8_t rslt);

        /** @brief Get error value by reading the BHY2 error register */
        uint8_t get_bhy2_error_value();

        /** @brief Get and return the kernel version */
        uint16_t get_bhy2_kernel_version();

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
        
        /** @brief Flush the data from the virtual sensor on bhy*/
        void flushSensorData(uint8_t sensorID);
        
        /** @brief Send firmware to flash in parts*/
        int8_t upload_firmware_to_flash_partly(uint8_t *bhy2_firmware_image, uint32_t offset, uint32_t packet_size);

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