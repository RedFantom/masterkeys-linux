# Author: RedFantom
# License: GNU GPLv3
# Copyright (c) 2018-2019 RedFantom
# Source: https://eb2.co/blog/2012/03
find_program(SPHINX_EXECUTABLE NAMES sphinx-build
        HINTS $ENV{SPHINX_DIR} PATH_SUFFIXES BIN
        DOC "Sphinx Documentation Generator")
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Sphinx DEFAULT_MSG SPHINX_EXECUTABLE)
mark_as_advanced(SPHINX_EXECUTABLE)
