# Interfacing Arduino NiclaSenseME with nRFConnectSDK

The repository contains the firmware written using nRFConnectSDK and Zephyr RTOS for Arduino NiclaSenseME.


## Why was it done?

- Provides better control over the nrf52832 chip
- Customization
- Portability to custom PCBs designed for smart manufacturing applications containing the nrf52832 chip
  - Requires updates to the Device Tree file


## Contents

- nicla_basic &emsp;&emsp; -> Control the NiclaSenseME system and DAQ over BLE.
    - DAQ at 400Hz sampling rate from multiple sensors (3-axis acceleration and 3-axis orientation)
    - Control and sensor configuration over BLE
    - OTA firmware updates

## Acknowledgements

The classes and methods were designed to closely resemble the Arduino code for the Nicla Sense ME, ensuring compatibility and repeatability. The Arduino Nicla Sense ME firmware can be found at https://github.com/arduino/nicla-sense-me-fw.
