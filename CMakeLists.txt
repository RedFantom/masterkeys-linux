# Author: RedFantom
# License: GNU GPLv3
# Copyright (c) 2018-2019 RedFantom
cmake_minimum_required(VERSION 3.9)
set(CMAKE_C_FLAGS "-std=c99")
project(libmk VERSION 0.1.0 DESCRIPTION "MasterKeys RGB Library")

# Find LibUSB
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

# LibMK

add_library(mk SHARED libmk/libmk.c)
set_target_properties(mk PROPERTIES
    VERSION ${PROJECT_VERSION}
    PUBLIC_HEADER libmk/libmk.h
    PREFIX "")
target_link_libraries(mk usb-1.0)
include_directories(${LIBUSB_INCLUDE_DIR})

install(TARGETS mk
    LIBRARY DESTINATION lib
    PUBLIC_HEADER DESTINATION include)

if(SKBUILD)  # The masterkeys package can only be built with scikit-build

    # python-masterkeys
    project(masterkeys VERSION 0.1.0 DESCRIPTION "Wrapper around libmk")
    find_package(PythonInterp REQUIRED)
    find_package(PythonLibs REQUIRED)
    find_package(PythonExtensions)
    add_library(masterkeys MODULE
            masterkeys/masterkeys.c
            libmk/libmk.c
            libmk/libmk.h)


    python_extension_module(masterkeys)
    target_include_directories(masterkeys PUBLIC
            ${PYTHON_INCLUDE_DIRS}
            ./libmk)

    target_link_libraries(masterkeys ${PYTHON_LIBRARIES} mk usb-1.0)

    set_target_properties(masterkeys PROPERTIES
        OUTPUT_NAME "masterkeys"
        PREFIX "")

    install(TARGETS masterkeys LIBRARY DESTINATION masterkeys)

else()

    add_executable(main utils/main.c)
    add_executable(record utils/record.c)
    target_link_libraries(main usb-1.0 mk)
    target_link_libraries(record mk)


endif()
include(GNUInstallDirs)
