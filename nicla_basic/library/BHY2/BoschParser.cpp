#include "BoschParser.h"
#include "BoschSensortec.h"
#include "sensors/SensorID.h"
#include <zephyr/logging/log.h>


// Logging
LOG_MODULE_REGISTER(MBoschParser, CONFIG_SET_LOG_LEVEL);

void BoschParser::convertTime(uint64_t time_ticks, uint32_t *s, uint32_t *ns) {
    uint64_t timestamp = time_ticks;

    timestamp = timestamp * 15625;
    *s = (uint32_t)(timestamp / UINT64_C(1000000000));
    *ns = (uint32_t)(timestamp - ((*s) * UINT64_C(1000000000)));
}


void BoschParser::parseData(const struct bhy2_fifo_parse_data_info *fifoData, void *arg) {

    int8_t sz;

    sz = fifoData->data_size - 1;
    if(sz <= SENSOR_DATA_FIXED_LENGTH) {
        SensorDataPacket *sensorData = (SensorDataPacket *) k_malloc(sizeof(SensorDataPacket));
        sensorData->sensorId = fifoData->sensor_id;
        sensorData->size = sz + 1;
        if (sz > 0) 
            memcpy(sensorData->data, fifoData->data_ptr, sz);

        sensortec.addSensorData(sensorData);
    } else {
        SensorLongDataPacket *sensorDataLong = (SensorLongDataPacket *) k_malloc(sizeof(SensorLongDataPacket));
        sensorDataLong->sensorId = fifoData->sensor_id;
        sz = (sz <= SENSOR_LONG_DATA_FIXED_LENGTH) ? sz : SENSOR_LONG_DATA_FIXED_LENGTH;
        sensorDataLong->size = sz + 1;

        memcpy(sensorDataLong->data, fifoData->data_ptr, sz);
        sensortec.addLongSensorData(sensorDataLong);
    }

    // Debugging
    //LOG_DBG("Sensor: %u, Size: %u", fifoData->sensor_id, fifoData->data_size);

}


void BoschParser::parseMetaEvent(const struct bhy2_fifo_parse_data_info *callback_info, void *callback_ref) {
    uint8_t meta_event_type = callback_info->data_ptr[0];
    uint8_t byte1 = callback_info->data_ptr[1];
    uint8_t byte2 = callback_info->data_ptr[2];
    uint32_t s, ns;
    const char *event_text;

    if(callback_info->sensor_id == BHY2_SYS_ID_META_EVENT) {
        event_text = "[META EVENT]";

    } else if(callback_info->sensor_id == BHY2_SYS_ID_META_EVENT_WU) {
        event_text = "[META EVENT WAKE UP]";

    } else {
        return;
    }

    struct parse_ref *parse_table = (struct parse_ref*)callback_ref;
    (void)parse_table;


    switch (meta_event_type){
      case BHY2_META_EVENT_FLUSH_COMPLETE:
        LOG_DBG("%s Flush complete for sensor id %u", event_text, byte1);
        break;
      case BHY2_META_EVENT_SAMPLE_RATE_CHANGED:
        LOG_DBG("%s Sample rate changed for sensor id %u", event_text, byte1);
        break;
      case BHY2_META_EVENT_POWER_MODE_CHANGED:
        LOG_DBG("%s Power mode changed for sensor id %u", event_text, byte1);
        break;
      case BHY2_META_EVENT_ALGORITHM_EVENTS:
        LOG_DBG("%s Algorithm event", event_text);
        break;
      case BHY2_META_EVENT_SENSOR_STATUS:
        LOG_DBG("%s Accuracy for sensor id %u changed to %u", event_text, byte1, byte2);
        break;
      case BHY2_META_EVENT_BSX_DO_STEPS_MAIN:
        LOG_DBG("%s Algorithm event", event_text);
        break;
      case BHY2_META_EVENT_BSX_DO_STEPS_CALIB:
        LOG_DBG("%s BSX event (do steps calib)", event_text);
        break;
      case BHY2_META_EVENT_BSX_GET_OUTPUT_SIGNAL:
        LOG_DBG("%s BSX event (get output signal)", event_text);
        break;
      case BHY2_META_EVENT_SENSOR_ERROR:
        LOG_DBG("%s Sensor id %u reported error %u (DEC)", event_text, byte1, byte2);
        break;
      case BHY2_META_EVENT_FIFO_OVERFLOW:
        LOG_DBG("%s FIFO overflow", event_text);
        break;
      case BHY2_META_EVENT_DYNAMIC_RANGE_CHANGED:
        LOG_DBG("%s Dynamic range changed for sensor id %u", event_text, byte1);
        break;
      case BHY2_META_EVENT_FIFO_WATERMARK:
        LOG_DBG("%s FIFO watermark reached", event_text);
        break;
      case BHY2_META_EVENT_INITIALIZED:
        LOG_DBG("%s Firmware initialized. Firmware version %u", event_text, ((uint16_t )byte2 << 8) | byte1);
        break;
      case BHY2_META_TRANSFER_CAUSE:
        LOG_DBG("%s Transfer cause for sensor id %u", event_text, byte1);
        break;
      case BHY2_META_EVENT_SENSOR_FRAMEWORK:
        LOG_DBG("%s Sensor framework event for sensor id %u", event_text, byte1);
        break;
      case BHY2_META_EVENT_RESET:
        LOG_DBG("%s Reset event", event_text);
        break;
      case BHY2_META_EVENT_SPACER:
        break;
      default:
        LOG_DBG("%s Unknown meta event with id: %u", event_text, meta_event_type);
        break;
    }
}


void BoschParser::parseGeneric(const struct bhy2_fifo_parse_data_info *callback_info, void *callback_ref) {
    uint32_t s, ns;
    convertTime(*callback_info->time_stamp, &s, &ns);

    LOG_WRN("SID: %u; T: %u.%09u; ", callback_info->sensor_id, (unsigned int)s, (unsigned int)ns);
    for (uint8_t i = 0; i < (callback_info->data_size - 1); i++){
        LOG_WRN("%X ", callback_info->data_ptr[i]);
    }
    LOG_WRN("\r\n");
}


void BoschParser::parseDebugMessage(const struct bhy2_fifo_parse_data_info *callback_info, void *callback_ref) {
    LOG_DBG("[DEBUG MSG]: flag: %u, data: %s", callback_info->data_ptr[0], (char*)&callback_info->data_ptr[1]);
}
