import asyncio
import platform
import sys 
import time
import os
import struct
from tqdm import tqdm

from bleak import BleakClient
from bleak.uuids import uuid16_dict

# Device address
BLE_ADDRESS = (
    "F4F465DA-1B01-2E80-97F2-F60573E277F4"
)

# Firmware services
DFU_SERVICE_UUID = "34c2e3b8-34aa-11eb-adc1-0242ac120002"
DFU_INTERNAL_UUID = "34c2e3b8-34ab-11eb-adc1-0242ac120002"
DFU_EXTERNAL_UUID = "34c2e3b8-34ac-11eb-adc1-0242ac120002"


async def main(address, data):
    async with BleakClient(address, winrt=dict(use_cached_services=True)) as client:
        print(f"Connected: {client.is_connected}")

        # Preamble for data
        last_packet = 0
        buf = []
        transfer_bytes = 200

        # Packet information
        num_byte_packets = int(len(fw_var_uint8_t) / transfer_bytes)
        rm_bytes = len(fw_var_uint8_t) % transfer_bytes
        print(f"Total number of bytes {len(fw_var_uint8_t)}. Number of packets = {num_byte_packets}, Remainder packets = {rm_bytes}")

        # Transfer process
        for index in range(num_byte_packets):

            # Indexing bounds
            start_index = int(index * transfer_bytes)
            end_index = int(start_index + transfer_bytes)

            # When reaching the last byte group
            if index == (num_byte_packets - 1):
                if(rm_bytes == 0):
                    # Set the last packet
                    last_packet = 1

            print(start_index)

            # Data buffer
            buf = fw_var_uint8_t[start_index:end_index]
            await client.write_gatt_char(DFU_EXTERNAL_UUID, struct.pack( "<BL" + "B" * (transfer_bytes), last_packet, start_index, *buf), response=True)

        # Incase there are remainder bytes
        if rm_bytes != 0:
            
            # Get the indexes
            start_index = int(end_index)
            end_index = int(start_index + rm_bytes
)
            print(start_index)
            print(end_index)

            # Data buffer
            buf = fw_var_uint8_t[start_index:end_index]
            last_packet = 1
            await client.write_gatt_char(DFU_EXTERNAL_UUID, struct.pack( "<BI" + "B" * rm_bytes, last_packet, start_index, *buf), response=True)

if __name__ == "__main__":

    # Get the file path
    file_path = os.path.join(os.getcwd(), "host_src", "firmware", "fw.h")

    # Read the file data
    with open(file_path, "r") as file_handle:
        raw_data_str = ''
        for line in file_handle.readlines():
            raw_data_str += line

    # Separated variables
    raw_data_var_list = raw_data_str.split(";")
    print(f"Total number of variables: {len(raw_data_var_list)}")

    # Separate the variables into a dictionary
    var_data_dict = {}
    for var in raw_data_var_list:
        if var == "\n":
            continue
        var_name = var.split("=")[0].strip()
        var_data = var.split("=")[1].strip()

        var_name = var_name.replace("\n", " ").strip()
        var_data_dict[var_name] = var_data
    print(var_data_dict.keys())

    # Separate the variables
    fw_var = var_data_dict["const unsigned char BHI260AP_NiclaSenseME_flash_fw[]"]
    fw_byte_count = var_data_dict["unsigned int BHI260AP_NiclaSenseME_flash_fw_len"]

    fw_var_byte_list = []
    for byte_val in fw_var[1:-1].split(","):
        fw_var_byte_list.append(byte_val.strip())
    print(f"The length of byte list = {len(fw_var_byte_list)}\n\tThe actual count of byte list = {int(fw_byte_count)}")
    # Assertion
    assert len(fw_var_byte_list) == int(fw_byte_count), "The count does not match"

    # Convert to bytes
    fw_var_uint8_t = [int(x, 0) for x in fw_var_byte_list]
    crc = 0
    for val in fw_var_uint8_t:
        crc = crc ^ val
    print(f"The Computed CRC is {crc}")

    asyncio.run(main(BLE_ADDRESS, fw_var_uint8_t))     
