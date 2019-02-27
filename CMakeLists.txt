# Author: RedFantom
# License: GNU GPLv3
# Copyright (c) 2018-2019 RedFantom
cmake_minimum_required(VERSION 3.9)
set(CMAKE_C_FLAGS "-std=c99")
include(libmk/CMakeLists.txt)
if(SKBUILD)  # The masterkeys package can only be built with scikit-build
    include(masterkeys/CMakeLists.txt)
else()
    include(examples/CMakeLists.txt)
    include(utils/CMakeLists.txt)
endif()
include(GNUInstallDirs)
