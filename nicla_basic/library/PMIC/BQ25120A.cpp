#include <zephyr/kernel.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#include "BQ25120A.hpp"

// I2C device for PMIC
#define I2C0_BQ25120A DT_NODELABEL(bq25120a)
// GPIO controller
#define GPIO_NODEID DT_NODELABEL(pmiccd) 


// Logging module
LOG_MODULE_REGISTER(BQ25120A, LOG_LEVEL_WRN);

// BQ25120A I2C
static const struct i2c_dt_spec BQ25120A_i2c0 = I2C_DT_SPEC_GET(I2C0_BQ25120A);
// GPIO
struct gpio_dt_spec pmic_cd = GPIO_DT_SPEC_GET(GPIO_NODEID, gpios);


// Global variables 
uint8_t BQ25120A::_chg_reg = 0;


/*
Public Methods
*/
bool BQ25120A::enable3V3LDO() {

    // LS/LDO = 0.8V + LS_LDOCODE x 100mV
    uint8_t ldo_reg = 0xE4;
    // write reg
    writeByte(BQ25120A_LDO_CTRL, ldo_reg);
    uint8_t current_ldo_reg = {0};
    readByte(BQ25120A_LDO_CTRL, &current_ldo_reg, 1);
    if(current_ldo_reg != ldo_reg) {
        return false;
    }
    return true;

}


bool BQ25120A::enable1V8LDO() {
    
    // LS/LDO = 0.8V + LS_LDOCODE x 100mV
    uint8_t ldo_reg = 0xA8;
    // write reg
    writeByte(BQ25120A_LDO_CTRL, ldo_reg);
    uint8_t current_ldo_reg = {0};
    readByte(BQ25120A_LDO_CTRL, &current_ldo_reg, 1);
    if(current_ldo_reg != ldo_reg) {
        return false;
    }
    return true;

}


bool BQ25120A::disableLDO() {

    // read
    uint8_t ldo_reg = {0};
    readByte(BQ25120A_LDO_CTRL, &ldo_reg, 1);
    // Update the value
    ldo_reg &= 0x7F;
    writeByte(BQ25120A_LDO_CTRL, ldo_reg);
    uint8_t current_ldo_reg = {0};
    readByte(BQ25120A_LDO_CTRL, &current_ldo_reg, 1);
    if(current_ldo_reg != ldo_reg) {
        disableCD();
        return false;
    }
    return true;

}


uint8_t BQ25120A::getStatus(){

    uint8_t status_reg = {0};
    readByte(BQ25120A_STATUS, &status_reg, 1);
    return status_reg;

}


void BQ25120A::enterShipMode() {

    uint8_t status_reg = getStatus();
    status_reg |= 0x20;
    writeByte(BQ25120A_STATUS, status_reg);

}


bool  BQ25120A::enableCharge(uint8_t mA, bool disable_ntc) {

    // From Arduino
    if (mA < 5) {
        _chg_reg = 0x3;
    } else if (mA < 35) {
        _chg_reg = ((mA - 5) << 2);
    } else {
        _chg_reg = (((mA - 40) /10) << 2) | 0x80;
    }

    // Write the register
    writeByte(BQ25120A_FAST_CHG, _chg_reg);

    // Very depleted batteries
    writeByte(BQ25120A_ILIM_UVLO_CTRL, 0x3F);

    // Disable the TS and interrupt on charge
    if(disable_ntc) {
        writeByte(BQ25120A_TS_CONTROL, 1 << 3);
    }

    // Read back the current charge register
    uint8_t current_chrg_reg = {0};
    readByte(BQ25120A_FAST_CHG, &current_chrg_reg, 1);
    return current_chrg_reg == _chg_reg;

}


uint16_t BQ25120A::getFault() {

    uint8_t tmp1 = {0};
    uint8_t tmp2 = {0};
    readByte(BQ25120A_FAULTS, &tmp1, 1);
    readByte(BQ25120A_TS_CONTROL, &tmp2, 1);

    // Combine the faults
    uint16_t faults_reg = (((uint16_t) tmp1) << 8) | ((uint16_t) (tmp2 & 0x60)); 
    return faults_reg;
}


float BQ25120A::getBatteryVoltage() {

    // Initialize a reading
    writeByte(BQ25120A_BATT_MON, 0x80);
    k_msleep(3);
    // Read the voltage
    uint8_t batt_voltage = {0};
    readByte(BQ25120A_BATT_MON, &batt_voltage, 1);

    return 0.6f + (batt_voltage >> 5) * 0.1f + ((batt_voltage >> 2) & 0x7) * 0.02f;
}


uint8_t BQ25120A::getBatteryTemperature() {

    uint8_t ts_data = {0};
    readByte(BQ25120A_TS_CONTROL, &ts_data, 1);
    return (ts_data >> 5) & 0x03;
}


uint8_t BQ25120A::getChargingRegisterState() {

    uint8_t currentChgReg = {0};
    readByte(BQ25120A_FAST_CHG, &currentChgReg, 1);
    return currentChgReg;
}


/*
Private Methods
*/
// Enable Chip Disable
void BQ25120A::enableCD(){

    if(!device_is_ready(pmic_cd.port)) {
        LOG_WRN("GPIO0 port is not initialized\n");
    }

    // Configure PIN mode
    int ret = gpio_pin_configure_dt(&pmic_cd, GPIO_OUTPUT | GPIO_ACTIVE_HIGH);
    log_info(ret, "GPIO");
    ret = gpio_pin_set_dt(&pmic_cd, 1);

}


// Disable Chip Disable
void BQ25120A::disableCD() {

    if(!device_is_ready(pmic_cd.port)) {
        LOG_WRN("GPIO0 port is not initialized\n");
    }

    // Configure PIN model
    int ret = gpio_pin_configure_dt(&pmic_cd, GPIO_OUTPUT | GPIO_ACTIVE_HIGH);
    log_info(ret, "GPIO");
    ret = gpio_pin_set_dt(&pmic_cd, 0);
}


// Read LDO register
uint8_t BQ25120A::readLDOreg() {

    uint8_t read_buf = {0};
    readByte(BQ25120A_LDO_CTRL, &read_buf, 1);
    return read_buf;

}


void BQ25120A::checkChgReg() {

    // Read charging register
    uint8_t currentChgReg = {0};
    readByte(BQ25120A_FAST_CHG, &currentChgReg, 1);
    if(_chg_reg != currentChgReg) {
        writeByte(BQ25120A_FAST_CHG, _chg_reg);
    }

}


// Logging
void BQ25120A::log_info(int ret, const char* type) {
    if (ret != 0) {
        LOG_WRN("%s has failed\n", type);
    } else {
        LOG_INF("%s communication successful\n", type);
    }
}

/*
I2C Operations
*/
/*
@brief Writes a byte over I2C.

@note No need to send the address of the device. It is taken care by the DT and initialized from the DT overlay

@param subAddress      ->      Address to send over the I2C line to identify the register
@param data            ->      The 8-bit data that is to be written

@return None
*/
void BQ25120A::writeByte(uint8_t subAddress, uint8_t data) {

    // Enable device
    enableCD();

    // Device readiness
    if(!device_is_ready(BQ25120A_i2c0.bus)) {
        LOG_WRN("I2C device is not ready\n\r");
    }

    // Transfer buffer
    uint8_t buf[] = {subAddress, data};
    // I2C write
    int ret = i2c_write_dt(&BQ25120A_i2c0, buf, sizeof(buf));
    log_info(ret, "I2C");

    // Disable device
    disableCD();

}


/*
@brief Reads a byte over I2C
@note  No need to send the address of the device. It is taken care by the DT and initialized from the DT overlay

@param subAddress      ->      Address to send over the I2C line to identify the register
@param read_buf        ->      Buffer where the read data is to be placed
@param num_bytes       ->      Number of bytes to read from the device one after the other

@returns None
*/
void BQ25120A::readByte(uint8_t subAddress, uint8_t* read_buf, uint32_t num_bytes) {

    // Enable device
    enableCD();

    // Device readiness
    if(!device_is_ready(BQ25120A_i2c0.bus)) {
        LOG_WRN("I2C device is not ready\n\r");
    }

    uint8_t write_buf[] = {subAddress};
    // I2C write-read
    int ret = i2c_write_read_dt(&BQ25120A_i2c0, write_buf, sizeof(write_buf), read_buf, num_bytes);
    log_info(ret, "I2C");

    // Disable device
    disableCD();

}