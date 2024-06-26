#include "BoschSensortec.h"
#include "BoschParser.h"
#include "sensors/SensorManager.h"
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>


LOG_MODULE_REGISTER(Sensortec, CONFIG_SET_LOG_LEVEL);


BoschSensortec::BoschSensortec() :
    _acknowledgement(SensorNack)
{

}


BoschSensortec::~BoschSensortec()
{

}

void *operator new (size_t n) {
    void *const p = k_malloc(n);
    return p;
}

void operator delete(void *p) {
    k_free(p);
}

bool BoschSensortec::begin() {
    setup_interfaces(false, BHY2_SPI_INTERFACE);
    auto ret = bhy2_init(BHY2_SPI_INTERFACE, bhy2_spi_read, bhy2_spi_write, bhy2_delay_us, MAX_READ_WRITE_LEN, NULL, &_bhy2);
    LOG_DBG("bhy2_init: %s\n", get_api_error(ret));
    if(ret != BHY2_OK) return 1;

    ret = bhy2_soft_reset(&_bhy2);

    // Boot status
    uint8_t stat = 0;
    ret = bhy2_get_boot_status(&stat, &_bhy2);
    if(ret)
        LOG_DBG("ret = %s; Boot Status: %X\n", get_api_error(ret), stat);

    // Boot from flash
    ret = bhy2_boot_from_flash(&_bhy2);
    LOG_DBG("Boot from flash, ret = %s\n", get_api_error(ret));
    if(ret != BHY2_OK) return 1;

    // Boot status
    ret = bhy2_get_boot_status(&stat, &_bhy2);
    if(ret)
        LOG_DBG("ret = %s; Boot Status: %X\n", get_api_error(ret), stat);

    // Host interrupt ctrl

    ret = bhy2_get_host_interrupt_ctrl(&stat, &_bhy2);
    if(ret)
        LOG_DBG("ret = %s; Host interrupt ctrl register: %X\n", get_api_error(ret), stat);

    // Host interface ctrl
    ret = bhy2_get_host_intf_ctrl(&stat, &_bhy2);
    if(ret)
        LOG_DBG("ret = %s; Host interface ctrl register: %X\n", get_api_error(ret), stat);

    // FIFO parse callbacks for meta events
    bhy2_register_fifo_parse_callback(BHY2_SYS_ID_META_EVENT, BoschParser::parseMetaEvent, NULL, &_bhy2);
    bhy2_register_fifo_parse_callback(BHY2_SYS_ID_META_EVENT_WU, BoschParser::parseMetaEvent, NULL, &_bhy2);
    bhy2_register_fifo_parse_callback(BHY2_SYS_ID_DEBUG_MSG, BoschParser::parseDebugMessage, NULL, &_bhy2);

    // Data buffer for FIFO
    ret = bhy2_get_and_process_fifo(_workBuffer, WORK_BUFFER_SIZE, &_bhy2);
    if(ret)
        LOG_DBG("ret = %s; Processing data buffer for FIFO\n", get_api_error(ret));

    // FIFO parse callback for sensor data
    for(uint8_t i = 1; i < BHY2_SENSOR_ID_MAX; i++) {
        bhy2_register_fifo_parse_callback(i, BoschParser::parseData, NULL, &_bhy2);
    }

    // Update the callback table
    bhy2_update_virtual_sensor_list(&_bhy2);
    // Available virtual sensors
    bhy2_get_virt_sensor_list(_sensorsPresent, &_bhy2);

    // printSensors();

    return 0;

}

uint8_t BoschSensortec::get_boot_status() {
    uint8_t boot_status;

    int8_t rc = bhy2_get_boot_status(&boot_status, &_bhy2);
    print_api_error(rc);

    return boot_status;
}

int8_t BoschSensortec::erase_bhy2_flash(uint32_t start_addr, uint32_t end_addr) {
    int8_t rslt = bhy2_erase_flash(start_addr, end_addr, &_bhy2);
    return rslt;
}

int8_t BoschSensortec::update_host_interrupt_ctrl(uint8_t hintr_ctrl) {
    int8_t rslt = bhy2_set_host_interrupt_ctrl(hintr_ctrl, &_bhy2);
    return rslt;
}

int8_t BoschSensortec::update_host_interface_ctrl(uint8_t hif_ctrl) {
    int8_t rslt = bhy2_set_host_intf_ctrl(hif_ctrl, &_bhy2);
    return rslt;
}

int8_t BoschSensortec::soft_reset_bhy2_device() {
    int8_t rslt = bhy2_soft_reset(&_bhy2);
    return rslt;
}

void BoschSensortec::printSensors() {

    bool presentBuf[256];

    for(uint16_t i = 0; i < sizeof(_sensorsPresent); i++) {
        for(uint8_t j = 0; j < 8; j++) {
            presentBuf[i * 8 + j] = ((_sensorsPresent[i] >> j) & 0x01);
        }
    }

    LOG_DBG("Sensors Present: \n");
    for (int i = 0; i < (int)sizeof(presentBuf); i++) {
        LOG_DBG("%d - %s\n", i, get_sensor_name(i));
    }
}

void BoschSensortec::print_api_error(int8_t rslt) {
    if(rslt != BHY2_OK) {
        LOG_DBG("%s", get_api_error(rslt));
    }
}

uint8_t BoschSensortec::get_bhy2_error_value() {
    uint8_t bhy2_error;
    int8_t rslt = bhy2_get_error_value(&bhy2_error, &_bhy2);
    print_api_error(rslt);

    return bhy2_error;
}

uint16_t BoschSensortec::get_bhy2_kernel_version() {
    uint16_t kernel_version;
    int8_t rslt = bhy2_get_kernel_version(&kernel_version, &_bhy2);
    print_api_error(rslt);

    return kernel_version;
}

bool BoschSensortec::hasSensor(uint8_t sensorID) {
    int i = sensorID / 8;
    int j = sensorID % 8;
    return ((_sensorsPresent[i] >> j) & 0x01) == 1;
}

void BoschSensortec::configureSensor(SensorConfigurationPacket &config){
    auto ret = bhy2_set_virt_sensor_cfg(config.sensorId, config.sampleRate, config.latency, &_bhy2);
    if (ret == BHY2_OK) {
        _acknowledgement = SensorAck;
    } else {
        _acknowledgement = SensorNack;
        LOG_DBG("[FAIL] Sensor Configuration for ID - %u, Response - %s", config.sensorId, get_sensor_error_text(ret));
    }
}

int BoschSensortec::configureSensorRange(uint8_t id, uint16_t range) {
    auto ret = bhy2_set_virt_sensor_range(id, range, &_bhy2);
    if(ret == BHY2_OK) {
        return 0;
    } else {
        LOG_DBG("[FAIL] Sensor Range Configuration for ID - %u, Response - %s", id, get_sensor_error_text(ret));
    }
    return 1;
}

void BoschSensortec::getSensorConfiguration(uint8_t id, SensorConfig &virt_sensor_conf) {

    bhy2_get_virt_sensor_cfg(id, &virt_sensor_conf, &_bhy2);
}

uint8_t BoschSensortec::availableSensorData() {

    return !_sensorQueue.is_empty();
}

uint8_t BoschSensortec::availableLongSensorData() {

    return !_sensorLongQueue.is_empty();
}

bool BoschSensortec::readSensorData(SensorDataPacket *data) {

    return _sensorQueue.get(data);
}

bool BoschSensortec::readLongSensorData(SensorLongDataPacket *data) {
    
    return _sensorLongQueue.get(data);
}

void BoschSensortec::addSensorData(SensorDataPacket *sensorData) {
    
    /** @todo Fix the callback in BoschParser for malloc memory allocation*/
    int ret = _sensorQueue.put(sensorData);
    if(ret)
        LOG_DBG("[FAIL] Data cannot be added to Circular BUffer %d\n", ret);

    // Process the sensor data
    sensorManager.process(*sensorData);
}

void BoschSensortec::addLongSensorData(SensorLongDataPacket *sensorData) {
    int ret = _sensorLongQueue.put(sensorData);
    if(ret)
        LOG_DBG("[FAIL] Data cannot be added to Circular BUffer %d\n", ret);

    // Process the sensor data
    sensorManager.process(*sensorData);
}

void BoschSensortec::flushSensorData(uint8_t sensorID) {

    int8_t rc = bhy2_flush_fifo(sensorID, &_bhy2);
    if(rc)
        LOG_DBG("[FAIL] Flushing the data for the sensor with id %d has failed with %d\n", sensorID, rc);

}

uint8_t BoschSensortec::acknowledgement() {
    uint8_t ack = _acknowledgement;
    // Reset the acknowledgement
    _acknowledgement = SensorNack;
    return ack;
}

bool BoschSensortec::update() {
    if(get_interrupt_status()) {
        auto ret = bhy2_get_and_process_fifo(_workBuffer, WORK_BUFFER_SIZE, &_bhy2);
        
#if CONFIG_SET_LOG_LEVEL == 4
        if(ret) {
            LOG_DBG("ret - %s\n", get_api_error(ret));
        }
#endif
        
        return true;
    }

    return false;
}

int8_t BoschSensortec::upload_firmware_to_flash_partly(uint8_t *bhy2_firmware_image, uint32_t offset, uint32_t packet_len) {

    int8_t rc = bhy2_upload_firmware_to_flash_partly(bhy2_firmware_image, offset, packet_len, &_bhy2);
    return rc;

}

#ifdef __cplusplus
extern "C" {
#endif
#if BHY2_CFG_DELEGATE_FIFO_PARSE_CB_INFO_MGMT
void bhy2_get_fifo_parse_callback_info_delegate(uint8_t sensor_id, 
                        struct bhy2_fifo_parse_callback_table *info,
                        const struct bhy2_dev *dev) {

    info->callback_ref = NULL;
    if (sensor_id < BHY2_SENSOR_ID_MAX) {
        info->callback = BoschParser::parseData;
    } else {
        switch (sensor_id) {
            case BHY2_SYS_ID_META_EVENT:
            case BHY2_SYS_ID_META_EVENT_WU:
                info->callback = BoschParser::parseMetaEvent;
                break;
            case BHY2_SYS_ID_DEBUG_MSG:
                info->callback = BoschParser::parseDebugMessage;
                break;
            default:
                info->callback = NULL;
        }
    }
}
#endif

#ifdef __cplusplus
}
#endif

BoschSensortec sensortec;
