#ifndef _BQ25120A_H_
#define _BQ25120A_H_


// Regsiter Map
// https://www.ti.com/lit/ds/symlink/bq25120a.pdf?ts=1610608851953&ref_url=https%253A%252F%252Fwww.startpage.com%252F
#define BQ25120A_STATUS             0x00
#define BQ25120A_FAULTS             0x01
#define BQ25120A_TS_CONTROL         0x02
#define BQ25120A_FAST_CHG           0x03
#define BQ25120A_TERMINATION_CURR   0x04
#define BQ25120A_BATTERY_CTRL       0x05
#define BQ25120A_SYS_VOUT_CTRL      0x06
#define BQ25120A_LDO_CTRL           0x07
#define BQ25120A_PUSH_BUTT_CTRL     0x08
#define BQ25120A_ILIM_UVLO_CTRL     0x09
#define BQ25120A_BATT_MON           0x0A
#define BQ25120A_VIN_DPM            0x0B


class BQ25120A {

    public:
        BQ25120A() {};

        static uint8_t getStatus();
        static void enterShipMode();
        static bool enable3V3LDO();
        static bool enable1V8LDO();
        static bool disableLDO();
        static uint8_t readLDOreg();
        static bool enableCharge(uint8_t mA = 100, bool disable_ntc = true);
        static uint16_t getFault();
        static float getBatteryVoltage();
        static uint8_t getBatteryTemperature();
        static uint8_t getChargingRegisterState();

    private:
        static void enableCD();
        static void disableCD();
        static void checkChgReg();
        static void writeByte(uint8_t subAddress, uint8_t data);
        static void readByte(uint8_t subAddress, uint8_t* read_buf, uint32_t num_bytes);
        static void log_info(int ret, const char* type);
        static uint8_t _chg_reg;

};

#endif