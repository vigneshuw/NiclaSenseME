
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

## Serial output

For serial output use minicom

```shell
minicom -b 115200 -D /dev/cu.usbmodem90CFCB952
```

Note: The device name after the flag -D can be different depending on you pc. This is your device name or serial port to which the NiclaSenseME is connected on your PC.