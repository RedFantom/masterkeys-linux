# Author: RedFantom
# License: GNU GPLv3
# Copyright (c) 2018-2019 RedFantom
find_package(X11 REQUIRED)

if (X11_FOUND)
    include(examples/ambilight/CMakeLists.txt)
endif()
