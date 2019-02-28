# Author: RedFantom
# License: GNU GPLv3
# Copyright (c) 2018-2019 RedFantom
cmake_minimum_required(VERSION 3.9)
include(FindLibUSB.cmake)
find_package(X11 REQUIRED)
add_executable(ambilight examples/ambilight/ambilight.c)
message(${X11_INCLUDE_DIR})
message(${X11_LIBRARIES})
target_include_directories(ambilight PUBLIC
        ${X11_INCLUDE_DIR}
        ${LIBUSB_INCLUDE_DIR}
        libmk)
target_link_libraries(
        ambilight PUBLIC ${X11_LIBRARIES} mk usb-1.0 pthread)
