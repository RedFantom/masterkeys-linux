# MasterKeys Pro Interface Description
This document assumes at least some knowledge of how USB works. If you
do not understand the different transfer modes, interfaces and endpoints
described by the USB standard, it is recommended that you first spend
some time understanding the details of that.

## Device Descriptors
The Cooler Master `bVendor` is `0x2516`, which is the same for all
keyboards. However, each keyboard has a different `bDevice`, even
devices with the same name.

My own keyboard has a `bDevice` value of `0x003b`. However, the [USB
ID repository](https://usb-ids.gowdy.us/read/UD/2516) also reports
`0x0047` as a `bDevice` for the `MasterKeys Pro L`.

**Revisions and switches**

Personally, I would guess that the differences in `bDevice` values stem
from two things. First off, there are at least two revisions of the
`MasterKeys Pro L` keyboard. The first revision was prone to [having
some issues with specific keys](https://www.reddit.com/r/MechanicalKeyboards/comments/4ui8v9/help_cooler_master_masterkeys_pro_l_nonresponsive/).
The second revision (as I can attest to) does not have these issues.

Second, there might be different device ids for the keyboards with
different layouts. There are two main layouts available (independent of
QWERTY OR QWERTZ): US (ANSI) and ISO (EU). My device is ANSI.

**Library Implementation**

This library does not implement device detection by using the vendor
and device ids, but instead loops over a list of the devices and checks
if the descriptor strings (`iManufacturer` and `iProduct`) contain the
identifying marks of a `MasterKeys` keyboard.

## Connection
The MasterKeys keyboard present themselves to the system with three
interfaces, each of them recognized as a HID device. These interfaces
are:
```markdown
- Interface 0: HID Device
  EndPoint 1  IN,  8 byte packets, 0x81
- Interface 1: HID Device
  EndPoint 3  IN, 64 byte packets, 0x83
  EndPoint 4 OUT, 64 byte packets, 0x04
- Interface 2: HID Device
  EndPoint 2  IN, 64 byte packets, 0x82
```

It is the second interface, numbered 1, with the endpoints 3 and 4 that 
is used for LED control. The commands should be written in 64 byte
packets to EndPoint 4, return data is received on EndPoint 4. This data
can be the data stored in the keyboard itself and confirmation messages.

## Packets
All packets must be 64 bytes in size. The packets are created in the 
library using the `libmk_build_packet` function. At the top of `libmk.c`
is a list of packet headers (first byte) and opcodes (second byte).
There ought to be a pattern discoverable in the headers and opcodes,
but so far it is unclear what each and exact bit does. Further analysis
of the protocol with more instructions may yield more insight.

**Disclaimer**: Following is a list of packet descriptions. Note that 
these were written based on the `MasterKeys Pro L ANSI`, and that 
different keyboards, particularly with only monochrome LEDs, may work
differently.

#### Enable
Enable keyboard LED control. Control is not automatically released. Sets
the currently active LED profile to be altered.
```hex
41 02 00 ...
```

#### Disable
Release keyboard LED control. Automatically released (and then 
reclaimed) when a new Enable packet is received.
```hex
41 00 00 ...
```

#### Flush
Flush the keyboard color data that is in cache on the chip to the 
actual LED controllers. Used in `libmk_set_all_led_color`.
```hex
50 55 00 ...
```

#### Effects
**Simple**:
An effect can simply be enabled based on the last given settings 
(generally default) using the 'simple' effect setting method.
```hex
41 01 00 ...

51 28 00 00 xx 00 ...
xx: Effect number (enum LibMK_Effect)
```
The first packet somehow puts the keyboard into a different mode. The
exact purpose is currently unknown. The second packet sets the actual 
effect.

**Detailed**:
Effects can take additional arguments through specific bytes.
```hex
51 28 00 00 nn ss dd aa FF FF fr fg fb br bg bb FF FF ...
nn: Effect number (enum LibMK_Effect)
ss: Effect speed
dd: Effect direction (00, 02, 04, 06, %8)
aa: Effect amount (purpose differs wildly)
f*: Foreground color r, g, b
b*: Background color r, g, b
The rest of the packet is filled with FF.
```
 
#### All LED colors
Effect `LIBMK_EFF_CUSTOM` should be enabled before sending these
packets. The amount of packets (probably) depends on the size of the
keyboard. The offset conversion tables are used to build the packets.
Each packet contains the data for `LIBMK_ALL_LED_PER_PCK` LEDs. For 
a full keyboard, that is `LIBMK_ALL_LED_PCK_NUM` packets.
```hex
51 A8 xx 00 00 yy yy yy ... 00 00 00

xx: Offset of this package: 2 * (key offset % LIBMK_ALL_PER_PCK)
    00, 02, 04 ... etc.
yy: 3 element color values for 16 LEDs: rr gg bb rr gg bb ... etc.
The rest of the packet is filled up with zeros.
```

#### Single LED color
Currently lights only a single LED on the keyboard (so even if 
`libmk_set_single_led` is called multiple times, the previously lit
LEDs will turn off once a new one is turned on). There should be a way
to allow setting the color of multiple LEDs this way, but that it is
as of yet unclear how to achieve this.
```hex
C0 01 01 00 oo rr gg bb 00 00 ...
oo: Key offset
```

## TODO-list
- Determine how to read effect profiles from the keyboard
