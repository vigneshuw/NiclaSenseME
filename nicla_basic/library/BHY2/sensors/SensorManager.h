#ifndef _SENSOR_MANAGER_H_
#define _SENSOR_MANAGER_H_

#include "SensorClass.h"

class SensorManager {
    public:
        SensorManager();
        void process(SensorLongDataPacket &data);
        void process(SensorDataPacket &data);

        void subscribe(SensorClass *sensor);
        void unsubscribe(SensorClass *sensor);

    private:
        SensorClass *_sensors[10]; //Array of 256 or list to handle unsubscription
        int _sensorsLen;

};

extern SensorManager sensorManager;

#endif
