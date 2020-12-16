# Author: RedFantom
# License: GNU GPLv3
# Copyright (c) 2018-2019 RedFantom
cmake_minimum_required(VERSION 3.9)
set(CMAKE_C_FLAGS "-std=c99" CACHE STRING "compiler flags")
project(libmk VERSION 0.3.0 DESCRIPTION "MasterKeys RGB Library")

# Dependencies
set(LIBUSB_INCLUDE_DIR /usr/include/libusb-1.0)
FIND_PATH(LIBUSB_INCLUDE_DIR libusb.h
        HINTS $ENV{LIBUSB_ROOT}
        PATHS ${PC_LIBUSB_INCLUDEDIR} ${PC_LIBUSB_INCLUDE_DIRS}
        PATH_SUFFIXES include)
FIND_LIBRARY(LIBUSB_LIBRARIES NAMES usb-1.0
        HINTS $ENV{LIBUSB_ROOT}
        PATHS ${PC_LIBUSB_LIBDIR} ${PC_LIBUSB_LIBRARY_DIRS}
        PATH_SUFFIXES lib)
message("CMake found libusb: " ${LIBUSB_INCLUDE_DIR} ", usb-1.0")
find_package(X11 REQUIRED)

include_directories(${LIBUSB_INCLUDE_DIR} ${X11_INCLUDE_DIRS} libmk)
link_libraries(usb-1.0 ${X11_LIBRARIES})

# libmk library
add_library(mk SHARED libmk/libmk.c)
set_target_properties(mk PROPERTIES
    VERSION ${PROJECT_VERSION}
    PUBLIC_HEADER libmk/libmk.h)
add_library(mkc SHARED libmk/libmkc.c)
set_target_properties(mkc PROPERTIES
    VERSION ${PROJECT_VERSION}
    PUBLIC_HEADER libmk/libmkc.h)
target_link_libraries(mkc mk)
install(TARGETS mk
    LIBRARY DESTINATION lib
    PUBLIC_HEADER DESTINATION include)
install(TARGETS mkc
    LIBRARY DESTINATION lib
    PUBLIC_HEADER DESTINATION include)

# utils
add_executable(main utils/main.c)
target_link_libraries(main mk)
add_executable(record utils/record.c)
target_link_libraries(record mk)
add_executable(ctrl utils/ctrl.c)
target_link_libraries(ctrl mk mkc)

# examples
add_executable(ambilight examples/ambilight/ambilight.c)
target_link_libraries(ambilight mk pthread)

# masterkeys Python module
if (SKBUILD)   # python setup.py
    find_package(PythonInterp REQUIRED)
    find_package(PythonLibs REQUIRED)
    find_package(PythonExtensions)

    message("CMake found Python: " ${PYTHON_LIBRARIES})

    project(masterkeys VERSION 0.3.0 DESCRIPTION "Wrapper around libmk")
    add_library(masterkeys MODULE
        masterkeys/masterkeys.c
        libmk/libmk.c libmk/libmk.h)
    python_extension_module(masterkeys)
    target_link_libraries(masterkeys ${PYTHON_LIBRARIES})
    set_target_properties(masterkeys PROPERTIES
        OUTPUT_NAME "masterkeys")
    install(TARGETS masterkeys LIBRARY DESTINATION masterkeys)

    project(mk_notifications VERSION 0.3.0)
    add_library(mk_notifications MODULE
        examples/notifications/mk_notifications.c
        libmk/libmk.c libmk/libmk.h)
    target_link_libraries(mk_notifications ${PYTHON_LIBRARIES} mk)
    set_target_properties(mk_notifications PROPERTIES
        OUTPUT_NAME "mk_notifications")
endif()
