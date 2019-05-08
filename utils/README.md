# Utilities

## main
The program `main.c` is the testing executable for the functionality
of `libmk`. Run this to check if your keyboard is properly supported.
Tests include setting full keyboard color, effects, individual LED
control and profile control. The profile in slot 4 will be overwritten!

## ctrl
The program `ctrl.c` is the testing executable for the functionality
of `libmkc` and shows of the performance of the Cooler Master MasterKeys
RGB keyboards using a high speed, high refresh-rate software defined
color wave. `libmkc` works asynchronously to allow programs to achieve
such fast effects.

## record
The program `record.c` is used for adding support for new layouts for
devices with an already known protocol. Unfortunately, supporting
keyboards with a different protocol requires significant work in packet
sniffing to reverse engineer the protocol used.
