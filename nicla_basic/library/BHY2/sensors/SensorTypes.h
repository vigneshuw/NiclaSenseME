#ifndef _SENSOR_TYPES_H_
#define _SENSOR_TYPES_H_

#include <zephyr/types.h>
#include "../bosch/bhy2_defs.h"

#define SENSOR_DATA_FIXED_LENGTH                (10)
#define SENSOR_LONG_DATA_FIXED_LENGTH           (18)


// Virtual Sensor for BHI260AP
typedef bhy2_virt_sensor_conf SensorConfig;

struct __attribute__ ((packed)) SensorConfigurationPacket {
    uint8_t sensorId;
    // Sampling rate for sensor; Set 0 to disable
    float sampleRate;
    // Delay before sending notification via interrupt
    uint32_t latency;
};

struct __attribute__ ((packed)) SensorDataPacket {
    uint8_t sensorId;
    uint8_t size;
    uint8_t data[SENSOR_DATA_FIXED_LENGTH];

    float getFloat(uint8_t index) {
        float result = 0;
        uint8_t length = sizeof(result);
        if(index  + length > SENSOR_DATA_FIXED_LENGTH) {
            // Overflow safeguard
            length = SENSOR_DATA_FIXED_LENGTH > index ? SENSOR_DATA_FIXED_LENGTH - index : 0;
        }
        if (length > 0) 
            memcpy(&result, &data[index], length);
        return result;
    }

    uint8_t getUint8(uint8_t index) {
        if(index >= SENSOR_DATA_FIXED_LENGTH) {
            return 0;
        }
        return data[index];
    };

    uint16_t getUint16(uint8_t index) {
        uint16_t result = 0;
        uint8_t length = sizeof(result);
        if(index + length > SENSOR_DATA_FIXED_LENGTH) {
            length = SENSOR_DATA_FIXED_LENGTH > index ? SENSOR_DATA_FIXED_LENGTH - index : 0;
        }
        if (length > 0)
            memcpy(&result, &data[index], length);
        return result;
    }

    uint32_t getUint24(uint8_t index) {
        uint32_t result = 0;
        uint8_t length = 3;
        if (index + length > SENSOR_DATA_FIXED_LENGTH) {
            length = SENSOR_DATA_FIXED_LENGTH > index ? SENSOR_DATA_FIXED_LENGTH - index : 0;
        }
        if(length > 0)
            memcpy(&result, &data[index], length);
        return result;
    }

    uint32_t getUint32(uint8_t index) {
        uint32_t result = 0;
        uint8_t length = sizeof(result);
        if(index + length > SENSOR_DATA_FIXED_LENGTH) {
            length = SENSOR_DATA_FIXED_LENGTH > index ? SENSOR_DATA_FIXED_LENGTH - index : 0;
        }
        if(length > 0) 
            memcpy(&result, &data[index], length);
        return result;
    }

    int8_t getInt8(uint8_t index) {
        if (index >= SENSOR_DATA_FIXED_LENGTH) {
        return 0;
        }
        return data[index];
    }

    int16_t getInt16(uint8_t index) {
        int16_t result = 0;
        uint8_t length = sizeof(result);
        if (index + length > SENSOR_DATA_FIXED_LENGTH) {
        length = SENSOR_DATA_FIXED_LENGTH > index ? SENSOR_DATA_FIXED_LENGTH - index : 0;
        }
        if (length > 0)
            memcpy(&result, &data[index], length);
        return result;
    }

    int32_t getInt32(uint8_t index) {
        uint32_t result = 0;
        uint8_t length = sizeof(result);
        if(index + length > SENSOR_DATA_FIXED_LENGTH) {
            length = SENSOR_DATA_FIXED_LENGTH > index ? SENSOR_DATA_FIXED_LENGTH - index : 0;
        }
        if (length > 0) 
            memcpy(&result, &data[index], length);
        return result;
    }
};

struct __attribute__ ((packed)) SensorLongDataPacket {
    uint8_t sensorId;
    uint8_t size;
    uint8_t data[SENSOR_LONG_DATA_FIXED_LENGTH];

    float getFloat(uint8_t index) {
        float result = 0;
        uint8_t length = sizeof(result);
        if (index + length > SENSOR_LONG_DATA_FIXED_LENGTH) {
        //to safe guard against overflow
        length = SENSOR_LONG_DATA_FIXED_LENGTH > index ? SENSOR_LONG_DATA_FIXED_LENGTH - index : 0;
        }
        if (length > 0)
            memcpy(&result, &data[index], length);
        return result;
    }

    uint8_t getUint8(uint8_t index) {
        if (index >= SENSOR_LONG_DATA_FIXED_LENGTH) {
        return 0;
        }
        return data[index];
    }

    uint16_t getUint16(uint8_t index) {
        uint16_t result = 0;
        uint8_t length = sizeof(result);
        if (index + length > SENSOR_LONG_DATA_FIXED_LENGTH) {
        length = SENSOR_LONG_DATA_FIXED_LENGTH > index ? SENSOR_LONG_DATA_FIXED_LENGTH - index : 0;
        }
        if (length > 0)
            memcpy(&result, &data[index], length);
        return result;
    }

    uint32_t getUint24(uint8_t index) {
        uint32_t result = 0;
        uint8_t length = 3;
        if (index + length > SENSOR_LONG_DATA_FIXED_LENGTH) {
        length = SENSOR_LONG_DATA_FIXED_LENGTH > index ? SENSOR_LONG_DATA_FIXED_LENGTH - index : 0;
        }
        if (length > 0)
            memcpy(&result, &data[index], length);
        return result;
    }

    uint32_t getUint32(uint8_t index) {
        uint32_t result = 0;
        uint8_t length = sizeof(result);
        if (index + length > SENSOR_LONG_DATA_FIXED_LENGTH) {
        length = SENSOR_LONG_DATA_FIXED_LENGTH > index ? SENSOR_LONG_DATA_FIXED_LENGTH - index : 0;
        }
        if (length > 0)
            memcpy(&result, &data[index], length);
        return result;
    }

    int8_t getInt8(uint8_t index) {
        if (index >= SENSOR_LONG_DATA_FIXED_LENGTH) {
        return 0;
        }
        return data[index];
    }

    int16_t getInt16(uint8_t index) {
        int16_t result = 0;
        uint8_t length = sizeof(result);
        if (index + length > SENSOR_LONG_DATA_FIXED_LENGTH) {
        length = SENSOR_LONG_DATA_FIXED_LENGTH > index ? SENSOR_LONG_DATA_FIXED_LENGTH - index : 0;
        }
        if (length > 0)
            memcpy(&result, &data[index], length);
        return result;
    }

    int32_t getInt32(uint8_t index) {
        int32_t result = 0;
        uint8_t length = sizeof(result);
        if (index + length > SENSOR_LONG_DATA_FIXED_LENGTH) {
        length = SENSOR_LONG_DATA_FIXED_LENGTH > index ? SENSOR_LONG_DATA_FIXED_LENGTH - index : 0;
        }
        if (length > 0)
            memcpy(&result, &data[index], length);
        return result;
    }    
};


#endif
