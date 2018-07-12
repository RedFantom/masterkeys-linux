"""
Author: RedFantom
License: GNU GPLv3
Copyright (c) 2018 RedFantom
"""
try:
    from skbuild import setup
except ImportError:
    print("scikit-build is required to build this project")
    raise


setup(
    name="masterkeys",
    version="0.1.0",
    description="MasterKeys Control Library",
    packages=["masterkeys"],
    zip_safe=False
)
