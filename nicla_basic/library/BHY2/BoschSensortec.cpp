#include "BoschSensortec.h"
// #include "BoschParser.h"
#include "sensors/SensorManager.h"
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>


LOG_MODULE_REGISTER(BoschSensortech, LOG_LEVEL_DBG);


BoschSensortec::BoschSensortec() :
    _acknowledgement(SensorAck)
{

}


// BoschSensortec::~BoschSensortec()
// {

// }


bool BoschSensortec::begin() {
    setup_interfaces(false, BHY2_SPI_INTERFACE);
    auto ret = bhy2_init(BHY2_SPI_INTERFACE, bhy2_spi_read, bhy2_spi_write, bhy2_delay_us, MAX_READ_WRITE_LEN, NULL, &_bhy2);
    LOG_DBG("bhy2_init: %s\n", get_api_error(ret));
    if(ret != BHY2_OK) return false;

    ret = bhy2_soft_reset(&_bhy2);
    LOG_DBG("ret = %s;\n", get_api_error(ret));

    // Get the product ID
    uint8_t product_id;
    ret = bhy2_get_product_id(&product_id, &_bhy2);
    printk("The product ID obtained was %u\n", product_id);
    LOG_DBG("ret = %s; Product ID: %u\n", get_api_error(ret), product_id);

    // Print BHI status
    // uint8_t stat = 0;
    // ret = bhy2_get_boot_status(&stat, &_bhy2);
    // LOG_DBG("ret = %s; Boot Status: %X\n", get_api_error(ret), stat);

    // // Enable boot
    // ret = bhy2_boot_from_flash(&_bhy2);
    // LOG_DBG("Boot from flash, ret = %s\n", get_api_error(ret));
    // if(ret != BHY2_OK) return false;

    // // Boot Status again
    // ret = bhy2_get_boot_status(&stat, &_bhy2);
    // LOG_DBG("ret = %s; Boot Status: %X\n", get_api_error(ret), stat);

    // // Host interrupt ctrl
    // ret = bhy2_get_host_interrupt_ctrl(&stat, &_bhy2);
    // LOG_DBG("ret = %s; Host interrupt ctrl register: %X\n", get_api_error(ret), stat);

    // // Host interface ctrl
    // ret = bhy2_get_host_intf_ctrl(&stat, &_bhy2);
    // LOG_DBG("ret = %s; Host interface ctrl register: %X\n", get_api_error(ret), stat);

    // bhy2_register_fifo_parse_callback(BHY2_SYS_ID_META_EVENT)

    return false;

}

BoschSensortec sensortec;
