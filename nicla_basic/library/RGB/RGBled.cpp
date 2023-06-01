#include "RGBled.hpp"

#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>


#define I2C0_RGBled DT_NODELABEL(rgbled)


LOG_MODULE_REGISTER(RGBLed, CONFIG_SET_LOG_LEVEL);

// RGBled device structure
static const struct i2c_dt_spec RGBled_i2c0 = I2C_DT_SPEC_GET(I2C0_RGBled);

void RGBled::begin() {

    reset();
    init();
    powerUp();

}

void RGBled::end() {

    powerDown();

}

void RGBled::setColor(RGBColors color) {
    
    if (color == off) {
        _blue = 0x00;
        _green = 0x00;
        _red = 0x00;
    }

    if(color == green) {
        _blue = 0x00;
        _green = 0xFF;
        _red = 0x00;
    }

    if(color == blue) {
        _blue = 0xFF;
        _green = 0x00;
        _red = 0x00;
    }

    if(color == red) {
        _blue = 0x00;
        _green = 0x00;
        _red = 0xFF;
    }

    if(color == cyan) {
        _blue = 0x20;
        _green = 0x20;
        _red = 0x00;
    }

    if(color == magenta) {
        _blue = 0x20;
        _green = 0x00;
        _red = 0x20;
    }

    if(color == yellow) {
        _blue = 0x00;
        _green = 0x20;
        _red = 0x20;
    }

    // Set the color over I2C
    setColor(_red, _green, _blue);

}

void RGBled::setColorBlue(uint8_t blue) {

    // Blue LED
    writeByte(IS31FL3194_OUT1, blue >> scale_factor);
    writeByte(IS31FL3194_COLOR_UPDATE, 0xC5);

}

void RGBled::setColorRed(uint8_t red) {

    // Red LED
    writeByte(IS31FL3194_OUT3, red >> scale_factor);
    writeByte(IS31FL3194_COLOR_UPDATE, 0xC5);

}

void RGBled::setColorGreen(uint8_t green) {

    // Green LED
    writeByte(IS31FL3194_OUT2, green >> scale_factor);
    writeByte(IS31FL3194_COLOR_UPDATE, 0xC5);

}

void RGBled::setColor(uint8_t red, uint8_t green, uint8_t blue) {

    // set current for rgb led
    writeByte(IS31FL3194_OUT1, blue >> scale_factor); //maximum current
    writeByte(IS31FL3194_OUT2, green >> scale_factor);
    writeByte(IS31FL3194_OUT3, red >> scale_factor);
    writeByte(IS31FL3194_COLOR_UPDATE, 0xC5); // write to color update register for changes to take effect
}


void RGBled::ledBlink(RGBColors color, uint32_t duration) {

    setColor(color);
    k_msleep(duration);
    setColor(off);

}

void RGBled::reset() {
    
    writeByte(IS31FL3194_RESET, 0xC5);
}

void RGBled::init() {

    writeByte(IS31FL3194_OP_CONFIG, 0x01);     // normal operation in current mode
    writeByte(IS31FL3194_OUT_CONFIG, 0x07);    // enable all three ouputs
    writeByte(IS31FL3194_CURRENT_BAND, 0x00);  // 10 mA max current
    writeByte(IS31FL3194_HOLD_FUNCTION, 0x00); // hold function disable

}

void RGBled::powerUp() {

    // Read data buffer
    uint8_t data = {0};
    readByte(IS31FL3194_OP_CONFIG, &data, 1);
    // write config
    writeByte(IS31FL3194_OP_CONFIG, data | 0x01);   //set bit 0 to enable
}


void RGBled::powerDown() {

    // Read data buffer
    uint8_t data = {0};
    readByte(IS31FL3194_OP_CONFIG, &data, 1);
    // write config
    writeByte(IS31FL3194_OP_CONFIG, data & ~(0x01));   //set bit 0 to enable
}


void RGBled::getChipID(uint8_t* buf, uint32_t num_bytes) {

    readByte(IS31FL3194_PRODUCT_ID, buf, num_bytes);

}

void RGBled::writeByte(uint8_t subAddress, uint8_t data) {

    // Check for device readiness
    if(!device_is_ready(RGBled_i2c0.bus)) {
        LOG_WRN("Device is not ready!\n\r");
    }

    // Transfer buffer
    uint8_t buf[] = {subAddress, data};
    // I2C write
    int ret = i2c_write_dt(&RGBled_i2c0, buf, sizeof(buf));
    log_info(ret);
}

void RGBled::readByte(uint8_t subAddress, uint8_t* read_buf, uint32_t num_bytes) {
    // Check for device readiness
    if(!device_is_ready(RGBled_i2c0.bus)) {
        LOG_WRN("Device is not ready!\n\r");
    }

    uint8_t write_buf[] = {subAddress};
    // I2C write-read
    int ret = i2c_write_read_dt(&RGBled_i2c0, write_buf, sizeof(write_buf), read_buf, num_bytes);
    log_info(ret);

}

void RGBled::log_info(int ret) {
    if (ret != 0) {
        LOG_WRN("I2C has failed\n");
    }
}

