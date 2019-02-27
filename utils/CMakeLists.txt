# Author: RedFantom
# License: GNU GPLv3
# Copyright (c) 2018-2019 RedFantom
add_executable(main utils/main.c)
add_executable(record utils/record.c)
target_link_libraries(main usb-1.0 mk)
target_link_libraries(record mk)
