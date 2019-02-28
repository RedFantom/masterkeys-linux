# Author: RedFantom
# License: GNU GPLv3
# Copyright (c) 2018-2019 RedFantom
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
        ../libmk)

target_link_libraries(masterkeys ${PYTHON_LIBRARIES} mk usb-1.0)

set_target_properties(masterkeys PROPERTIES
    OUTPUT_NAME "masterkeys"
    PREFIX "")

install(TARGETS masterkeys LIBRARY DESTINATION masterkeys)
