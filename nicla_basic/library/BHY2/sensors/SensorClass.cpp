#include "SensorClass.h"
#include "SensorManager.h"


SensorClass::SensorClass():
    _id(0),
    _subscribed(false)
{

}


SensorClass::SensorClass(uint8_t id):
    _id(id),
    _subscribed(false)
{

}


SensorClass::~SensorClass() {
    end();
}


uint8_t SensorClass::id() {
    return _id;
}


bool SensorClass::begin(float rate, uint32_t latency) {
    if(sensortec.has )
}

