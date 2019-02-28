Device Support
==============

As Cooler Master Inc. has not provided any support, this library can
only support a limited amount of devices, specifically, it can only
support devices for which the record executable target has been
executed. This program uses the library to register an offset for each
key, which is required for the library to be able to control individual
keys. Effects and full lighting colors can be set regardless of these
offsets.

The current list of supported devices includes:

- MasterKeys Pro L RGB ANSI
- MasterKeys Pro S RGB ANSI (untested)
- MasterKeys Pro L RGB ISO (untested)

If you would like for your device to be supported as well, please run
the ``record`` executable.

Keyboards with only monochrome lighting may use a different protocol and
thus they would probably require more modifications than just adding a
key layout matrix. Do not hesitate to open an issue if you have a
monochrome keyboard, would like to see support and are willing to do
some USB packet sniffing.
