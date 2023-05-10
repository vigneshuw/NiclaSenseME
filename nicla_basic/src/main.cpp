#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/fs/fs.h>
#include <zephyr/drivers/uart.h>

#include <stdio.h>
#include <string.h>

#include "NiclaSystem.hpp"


// The littlefs file system
#define PARTITION_NODE              DT_NODELABEL(lfs1)


LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);


/*
UART 
*/
static uint8_t rx_buf[100];
const struct device *uart = DEVICE_DT_GET(DT_NODELABEL(uart0));
// UART Configuration
const struct uart_config uart_cfg = {
    .baudrate = 115200,
    .parity = UART_CFG_PARITY_NONE,
    .stop_bits = UART_CFG_STOP_BITS_1,
    .data_bits = UART_CFG_DATA_BITS_8,
    .flow_ctrl = UART_CFG_FLOW_CTRL_RTS_CTS
};
// CRC for checking
uint8_t crc = 0;
static bool led = false;

/** @brief Callback function for UART*/
static void uart_cb(const struct device *dev, struct uart_event *evt, void *user_data) {
    
    switch (evt->type) {
        case UART_TX_DONE:
            break;

        case UART_TX_ABORTED:
            break;

        case UART_RX_RDY:
            for(unsigned int i = 0; i < evt->data.rx.len; i++) {
                crc = crc ^ evt->data.rx.buf[i + evt->data.rx.offset];
            }

            break;

        case UART_RX_BUF_REQUEST:
            break;

        case UART_RX_BUF_RELEASED:
            break;

        case UART_RX_DISABLED:
            //Re-enable with same buffer
            uart_rx_enable(dev, rx_buf, sizeof(rx_buf), 100);
            break;

        case UART_RX_STOPPED:
            break;

        default:
            break;
    }
}


int main(void) {

    // Enable LEDs and charging
    nicla::leds.begin();
    nicla::pmic.enableCharge(100);

    LOG_INF("Uploading the BHI fimware through UART\n");

    // Check device status
    int err;
    if(!device_is_ready(uart)) {
        printk("UART device not ready\r\n");
        return 1;
    }
    // Configure UART
    err = uart_configure(uart, &uart_cfg);
    if(err == -ENOSYS){
        return -ENOSYS;
    }
    // Register Callback
    err = uart_callback_set(uart, uart_cb, NULL);
    if(err) {
        return err;
    }
    // Enable RX
    int ret = uart_rx_enable(uart, rx_buf, sizeof(rx_buf), 100);
    if(ret){
        return 1;
    }


    while (1) {
        k_msleep(1000);
        printk("The CRC %u\n", crc);
    }


}