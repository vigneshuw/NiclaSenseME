#ifndef _SENSOR_MANAGER_H_
#define _SENSOR_MANAGER_H_

#include "SensorClass.h"

/** @brief Manages all the intialized sensor through subscription and unsubscription. Can handle a total of 10 sensors*/
class SensorManager {
    public:
        SensorManager();

        /** @brief Set data (setData) for all the subscribed sensors 
         * 
         * @param   data                    Data packet (Long data) for the sensors used for the raw data
        */
        void process(SensorLongDataPacket &data);

        /** @brief Set data (setData) for all the subscribed sensors 
         * 
         * @param   data                    Data packet for the sensors used for the raw data
        */
        void process(SensorDataPacket &data);

        /** @brief Subscribe a SensorClass
         * 
         * @param   sensor                  The SensorClass that is to be subscribed
        */
        void subscribe(SensorClass *sensor);

        /** @brief Unsubscribe a SensorClass
         * 
         * @param   sensor                  The SensorClass that is to be unsubscribed
        */
        void unsubscribe(SensorClass *sensor);

    private:
        SensorClass *_sensors[10]; //Array of 256 or list to handle unsubscription
        int _sensorsLen;

};

extern SensorManager sensorManager;

#endif
