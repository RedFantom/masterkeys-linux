"""
Author: RedFantom
License: GNU GPLv3
Copyright (c) 2018-2019 RedFantom
"""
try:
    from skbuild import setup
    from skbuild.command.build import build
except ImportError:
    print("scikit-build is required to build this project")
    raise
from shutil import copyfile
import glob
import os


def read(file_name):
    with open(file_name) as fi:
        contents = fi.read()
    return contents


class BuildCommand(build):
    """Intercept the build command to copy modules"""
    def run(self):
        build.run(self)
        if len(glob.glob("./_skbuild/linux*")) != 0:
            source = os.path.join("./_skbuild/linux*/cmake-build/*notifications.so*")
        else:
            source = os.path.join("./_skbuild/cmake-build/*notifications.so*")
        target = os.path.join("./examples/notifications/mk_notifications.so")
        copyfile(glob.glob(source)[0], target)


setup(
    name="masterkeys",
    version="0.3.0",
    packages=["masterkeys"],
    description="MasterKeys Control Library for Linux",
    author="RedFantom",
    url="https://github.com/RedFantom/masterkeys-linux",
    download_url="https://github.com/RedFantom/masterkeys-linux/releases",
    license="GNU GPLv3",
    classifiers=[
        "Programming Language :: Python :: 2.7",
        "Programming Language :: Python :: 3",
        "Programming Language :: C",
        "Development Status :: 3 - Alpha",
        "Intended Audience :: Developers",
        "License :: OSI Approved :: GNU General Public License v3 (GPLv3)",
        "Operating System :: POSIX :: Linux",
        "Topic :: Software Development :: Libraries :: Python Modules",
        "Topic :: System :: Hardware",
    ],
    long_description=read("README.md"),
    long_description_content_type="text/markdown",
    zip_safe=False,
    install_requires=["scikit-build"],
    cmdclass={"build": BuildCommand}
)
