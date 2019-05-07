# Author: RedFantom
# License: GNU GPLv3
# Copyright (c) 2018-2019 RedFantom
project(mk_notifications VERSION "0.1.0")
find_package(PythonInterp REQUIRED)
find_package(PythonLibs REQUIRED)
find_package(PythonExtensions)
add_library(mk_notifications MODULE
        examples/notifications/mk_notifications.c)
python_extension_module(mk_notifications)

message(${CMAKE_CURRENT_LIST_DIR})
message(${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(masterkeys PUBLIC
        ${PYTHON_INCLUDE_DIRS}
        libmk)
target_link_libraries(masterkeys ${PYTHON_LIBRARIES} mk usb-1.0 pthread)

set_target_properties(masterkeys PROPERTIES
        OUTPUT_NAME "masterkeys"
        PREFIX "")
