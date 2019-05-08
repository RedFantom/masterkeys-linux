# MasterKeys Linux SDK
[![Build Status](https://travis-ci.com/RedFantom/masterkeys-linux.svg?token=UBcv5ZyxSrELyQhSpadq&branch=master)](https://travis-ci.com/RedFantom/masterkeys-linux)
[![License: GPL v3](https://img.shields.io/badge/License-GPL%20v3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![PyPI version](https://badge.fury.io/py/masterkeys.svg)](https://pypi.org/project/masterkeys/)
[![Documentation](https://readthedocs.org/projects/masterkeys-linux/badge/?version=latest)](https://masterkeys-linux.readthedocs.io/en/latest)

Cooler Master provides an SDK for its MasterKeys series of keyboards
for use under Windows, but not for Linux. The SDK communicates with an
OUT-endpoint in the USB HID-device of the keyboard. This library aims to
make this communication accessible from Linux by using libusb.

This is the first time I have written such a large project in C, let
alone an actual shared library, so you may spot things that are bad
practice, bugs or other issues. Please let me know in the issues section 
if you spot anything!

## Disclaimer
This library is not endorsed or supported by Cooler Master Inc. or any
of its affiliates. They have explicitly refused to help in the 
development of this library. There is no official support for this 
library from Cooler Master Inc. Any and all questions should be posted 
on the issues page.

## Device support
As Cooler Master Inc. has not provided any support, this library can 
only support a limited amount of devices, specifically, it can only 
support devices for which the `record` executable target has been
executed. This program uses the library to register an offset for each
key, which is required for the library to be able to control individual
keys. Effects and full lighting colors can be set regardless of these
offsets.

The current list of supported devices includes:
- MasterKeys Pro L RGB ANSI
- MasterKeys Pro S RGB ANSI (untested)
- MasterKeys Pro L RGB ISO (untested)

If you would like for your device to be supported as well, please run 
the `record` executable. Enter the row and column coordinates of each
key according to the Cooler Master reference each time for the key that
lights up in red. The shared `layout.c` can be attached to an issue, and
then your device is added in no-time!

Keyboards with only monochrome lighting may use a different protocol and
thus they would probably require more modifications than just adding a
key layout matrix. Do not hesitate to open an issue if you have a
monochrome keyboard, would like to see support and are willing to do
some USB packet sniffing.

## Compiling and installing
To be able to compile and install any of the targets in this library,
`cmake` and its dependencies are required. Depending on your specific
distribution, the name of the packages (if they are provided) may 
differ from the ones given here. The reference commands are for Ubuntu.
```bash
# libx11-dev is for the AmbiLight and notifications examples
# python3-gtk2.0 is for the notifications example
sudo apt-get install cmake libusb-1.0.0-dev libx11-dev
cd Source/masterkeys-linux  # Or wherever you have cloned the repo

# Builds library, utilities and C examples
# Exclude them from the file if you don't want them to be built
cmake .
make
sudo make install

# For the Python library (system-wide install) and Python examples
sudo python -m pip install scikit-build  # Needed for building
sudo python -m pip install PyGObject dbus-python # Notifications example
sudo python setup.py build install  # Python examples not installed

# Or if you would rather install from PyPI, still requires dependencies
sudo python -m pip install masterkeys
```

Wheels are not provided at this time because building `manylinux` wheels
requires a different environment. It is considered as a possibility 
though. Until that time, it is possible to use the normal `linux_x86_64`
wheels provided on the [releases-page](https://github.com/RedFantom/masterkeys-linux/releases).

## Contributing
Pull Requests and contributions in other forms (such as issue reports) 
as well as tips or possible improvements are very welcome! As mentioned,
this is my first C library, and any help is greatly appreciated! If it 
comes in the form of code, you will be credited for your work in the
copyright notice.

If you would like for your device to be supported, please read the 
`Device Support` section of this file.

Given the small size of this project, there is no code of conduct or 
`CONTRIBUTING.md` with guidelines, but keep things professional. Also,
use descriptive commit messages. Force pushing to forks while a PR is 
open is fine (as long as it does not completely remove the 
contributions).

## License
```license
MasterKeys Linux - C Library to control RGB keyboards
Copyright (C) 2018 RedFantom

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, version 3.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
```

## Credits
Part of the implementation of the `libmk` library is based on the more
extensive protocol description written by [`chmod222`](https://github.com/chmod222),
available under the LGPLv3 license in [`libcmmk`](https://github.com/chmod222/libcmmk),
which has the same goal as this project.
