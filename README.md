# Interfacing Arduino Nicla Sense ME with nRFConnectSDK

The repository contains the code to interface with an Arduino Nicla Sense ME using nRFConnectSDK and Zephyr RTOS.

**Note: The Bootloader is removed when you flash the program to the device**

## Why was it done?

- Provides better control over the nrf52832 chip
- Customization
- Portability to custom PCBs containing the nrf52832 chip
  - Requires updates to the Device Tree file


## Contents

- nicla_basic &emsp;&emsp;&emsp;&emsp; -> Basic control over the NiclaSenseME system and DAQ over BLE at a sampling rate of 400Hz.  
