# Author: RedFantom
# License: GNU GPLv3
# Copyright (c) 2018-2019 RedFantom

# LibUSB include directory on Linux
set(LIBUSB_INCLUDE_DIR /usr/include/libusb-1.0)

# Path finding for libusb
FIND_PATH(LIBUSB_INCLUDE_DIR libusb.h
    HINTS $ENV{LIBUSB_ROOT}
    PATHS ${PC_LIBUSB_INCLUDEDIR} ${PC_LIBUSB_INCLUDE_DIRS}
    PATH_SUFFIXES include)
FIND_LIBRARY(LIBUSB_LIBRARIES NAMES usb-1.0
    HINTS $ENV{LIBUSB_ROOT}
    PATHS ${PC_LIBUSB_LIBDIR} ${PC_LIBUSB_LIBRARY_DIRS}
    PATH_SUFFIXES lib)
