// BLE GATT for Firmware Update
gattFirmware = {
    service: ["34c2e3b8-34aa-11eb-adc1-0242ac120002"],
    characteristics: {
        internal_fw: "34c2e3b8-34ab-11eb-adc1-0242ac120002",
        external_fw: "34c2e3b8-34ac-11eb-adc1-0242ac120002",
        initiate_update: "34c2e3b8-34ad-11eb-adc1-0242ac120002"
    }
}

// BLE GATT for Sensor Control
gattSensorControl = {
    service: "",
    characteristics: {
        sensor_config: ""
    }
}

module.exports = {
    gattFirmware,
    gattSensorControl,
}