
# Interfacing NiclaSenseME


## Overview


Interfacing with NiclaSenseME with Zephyr RTOS


## Building and Running

1. Use nrfConnectSDK for vs-code.

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

Note: The device name after the flag -D can be different depending on you pc. This is your device name or serial port to which the NiclaSenseME is connected on your PC.