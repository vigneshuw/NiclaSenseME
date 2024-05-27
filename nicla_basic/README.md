
# Interfacing NiclaSenseME


## Overview

The firmware in this repository has been specifically modified to be compatible with the Arduino NicaSenseME, which is commercially available. Flashing this firmware will enable data collection and transmission via Bluetooth Low Energy (BLE). Any BLE-enabled device can be used to interact with the Arduino NicaSenseME, allowing you to control the data acquisition (DAQ) process and receive the collected data.

## Features

- The firmware was developed using nRF Connect SDK and Zephyr RTOS.
- It is designed for data acquisition (DAQ) and transmission over Bluetooth Low Energy (BLE).
- The firmware supports remote over-the-air (OTA) updates for both the nRF52832 and BHI260AP using MCUboot.
- The firmware enables data acquisition (DAQ) from a specific sensor and allows configuration of sensor parameters, such as the sampling rate.


## Building and Running

1. Use nrfConnectSDK for vs-code. Toolchain version v2.3.0

2. Flashing the NiclaSenseME - CMSIS-DAP Link
   1. Connect the device using USB
   2. Install pyocd 
```python
pyocd flash -t nrf52832 ./build/zephyr/zephyr.hex
```

3. In case where the bootloader (MCUBoot) is present
   1. Erase the entire chip
   2. Flash the `merged.hex` - It contains the bootloader and the main application
```python
pyocd flash -e chip -t nrf52832 ./build/zephyr/merged.hex
```

4. If device reset is required
```python
pyocd reset -t nrf52832
```

## Serial output

1. For serial output use minicom
   1. The Serial device `-D` can change depending on the type of device used
   2. Check for the right serial device `ls /dev/cu.*`
```shell
minicom -b 115200 -D /dev/cu.usbmodem90CFCB952
```

Note: The device name after the flag -D can be different depending on your scenario. This is your device name or serial port to which the NiclaSenseME is connected on your PC.