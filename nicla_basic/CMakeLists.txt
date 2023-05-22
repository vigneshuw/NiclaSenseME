# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(nicla_basic)

# includes
include_directories(library)
include_directories(library/BHY2)

# Source files
# file(GLOB_RECURSE BHY2_C_SRC CONFIGURE_DEPENDS "library/BHY2/**.c" "library/BHY2/**.cpp")
file(GLOB_RECURSE LIB CONFIGURE_DEPENDS "library/**.c" "library/**.cpp")

target_sources(app PRIVATE src/main.cpp)
# target_sources(app PRIVATE library/BLE/NiclaService.cpp)
# target_sources(app PRIVATE library/RGB/RGBled.cpp)
# target_sources(app PRIVATE library/PMIC/BQ25120A.cpp)
# target_sources(app PRIVATE library/SPIFLASH/MX25R1635F.cpp)
# target_sources(app PRIVATE library/NiclaSystem.cpp)
# BHI260AP 
# target_sources(app PRIVATE library/BHY2/bosch/common/common.cpp)
# target_sources(app PRIVATE library/BHY2/bosch/bhy2_hif.c)
# target_sources(app PRIVATE library/BHY2/bosch/bhy2.c)
# target_sources(app PRIVATE library/BHY2/bosch/bhy2_parse.c)
# target_sources(app PRIVATE library/BHY2/BoschSensortec.cpp)
target_sources(app PRIVATE ${LIB})
