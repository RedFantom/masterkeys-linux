# Author: RedFantom
# License: GNU GPLv3
# Copyright (c) 2018-2019 RedFantom

# libusb 1.0
find_path(LIBUSB_INCLUDE_DIR libusb.h
    PATHS /usr/include /usr/local/include
    PATH_SUFFIXES libusb-1.0 libusb usb)
find_library(LIBUSB_LIBRARIES NAMES usb-1.0
    PATHS /usr/lib /usr/local/lib
    PATH_SUFFIXES libusb-1.0 libusb usb)

# libnotify
find_path(LIBNOTIFY_INCLUDE_DIR notify.h
    PATHS /usr/include /usr/local/include
    PATH_SUFFIXES libnotify notify)
find_library(LIBNOTIFY_LIBRARIES NAMES notify
    PATHS /usr/lib /usr/local/lib
    PATH_SUFFIXES libnotify notify)

# glib
find_package(PkgConfig REQUIRED)
pkg_search_module(DBUS REQUIRED dbus-1)
