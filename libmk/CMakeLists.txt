# Author: RedFantom
# License: GNU GPLv3
# Copyright (c) 2018-2019 RedFantom
project(libmk VERSION 0.1.0 DESCRIPTION "MasterKeys RGB Library")
include(FindLibUSB.cmake)

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
