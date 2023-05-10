import asyncio
import platform
import sys 
import time
import os
import struct
from tqdm import tqdm

from bleak import BleakClient
from bleak.uuids import uuid16_dict

ADDRESS = (
    "F4F465DA-1B01-2E80-97F2-F60573E277F4"
)

SERVICE = "00001523-1212-efde-1523-785feabcd123"
READ_ID_UUID = "00001523-1213-efde-1523-785feabcd123"
WRITE_ID_UUID = "00001523-1215-efde-1523-785feabcd123"


async def main(address, data):
    async with BleakClient(address, winrt=dict(use_cached_services=True)) as client:
        print(f"Connected: {client.is_connected}")

        buf = []
        transfer_bytes = 240
        for byte_val in tqdm(fw_var_uint8_t):
            if(len(buf) < transfer_bytes):
                buf.append(byte_val)
                continue
            buf.append(byte_val)
            await client.write_gatt_char(WRITE_ID_UUID, struct.pack( "<" + "B" * (transfer_bytes + 1), *buf), response=True)
            buf = []

        if (len(buf) != 0):
            await client.write_gatt_char(WRITE_ID_UUID, struct.pack( "<" + "B" * len(buf), *buf), response=True)


        time.sleep(10)

if __name__ == "__main__":

    # Get the file path
    file_path = os.path.join(os.getcwd(), "firmware", "fw.h")

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



    asyncio.run(main(ADDRESS, fw_var_uint8_t))     
