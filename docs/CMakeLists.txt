# Author: RedFantom
# License: GNU GPLv3
# Copyright (c) 2018-2019 RedFantom
# Source: https://eb2.co/blog/2012/03
cmake_minimum_required(VERSION 3.9)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}")
find_package(Sphinx REQUIRED)
if(NOT DEFINED SPHINX_THEME)
    set(SPHINX_THEME readthedocs)
endif()
if(NOT DEFINED SPHINX_THEME_DIR)
    set(SPHINX_THEME_DIR)
endif()

set(BINARY_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}")
set(SPHINX_CACHE_DIR "${CMAKE_CURRENT_BINARY_DIR}/_doctrees")
set(SPHINX_HTML_DIR "${CMAKE_CURRENT_BINARY_DIR}/html")

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/conf.py"
    "${BINARY_BUILD_DIR}/conf.py"
    @ONLY)
add_custom_target(libmk_docs
        ${SPHINX_EXECUTABLE}
            -q -b html
            -c "${BINARY_BUILD_DIR}"
            -d "${SPHINX_CACHE_DIR}"
            "${CMAKE_CURRENT_SOURCE_DIR}"
            "${SPHINX_HTML_DIR}"
        COMMENT "Building HTML documentation")
