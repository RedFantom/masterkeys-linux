# Author: RedFantom
# License: GNU GPLv3
# Copyright (c) 2018 RedFantom
cmake_minimum_required(VERSION 3.9)
project(libmkc VERSION 0.1.0 DESCRIPTION "MasterKeys RGB Controller Library")
include(FindLibUSB.txt)

add_library(mkc SHARED libmkc/libmkc.c)
set_target_properties(mkc PROPERTIES
        VERSION ${PROJECT_VERSION}
        PUBLIC_HEADER libmkc/libmkc.h
        PREFIX "")
target_link_libraries(mk usb-1.0)
include_directories(${LIBUSB_INCLUDE_DIR})

install(TARGETS mkc
        LIBRARY DESTINATION lib
        PUBLIC_HEADER DESTINATION include)
