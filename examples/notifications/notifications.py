"""
Author: RedFantom
License: GNU GPLv3
Copyright (c) 2019 RedFantom
"""
import gi
gi.require_version("GdkPixbuf", "2.0")
from gi.repository import GLib as glib
from gi.repository import GdkPixbuf as pixbuf
import mk_notifications
import time
import dbus
from dbus.mainloop.glib import DBusGMainLoop
import traceback
import numpy
from PIL import Image


def notifications(bus, message):
    try:
        handle_notification(message)
    except Exception:
        print("Exception in notification handler!")
        print(traceback.format_exc())


def handle_notification(message):
    for arg in message.get_args_list():
        if type(arg) == dbus.Dictionary:
            arg = {str(k): v for k, v in arg.items()}
            print(list(arg.keys()))
            if "image-data" not in arg or len(arg["image-data"]) != 7:
                print(len(arg), list(arg.keys()))
                print("Invalid message:", message)
                continue
            w, h, r, a, b, d, data = arg["image-data"]
            data = bytes([int(b) for b in data])
            mode = "RGB" if not a else "RGBA"
            image = Image.frombytes(
                mode, (w, h), data, "raw", mode, r)
            if mode == "RGBA":
                image = image.convert("RGB")
            array = numpy.array(image)
            data = [[tuple(int(e) for e in column) for column in row] for row in array]
            color = mk_notifications.calculate_dominant_color(
                data, 1, len(data), len(data[0]), 25, 700, 60, 1)
            mk_notifications.flash_keyboard(*color)


if __name__ == '__main__':
    r = mk_notifications.init(2, 25, 700, 60, 1, 20.0, 2, .5)
    if r is None:
        raise RuntimeError()
    mk_notifications.start()

    DBusGMainLoop(set_as_default=True)
    bus = dbus.SessionBus()
    bus.add_match_string_non_blocking("eavesdrop=true, interface='org.freedesktop.Notifications', member='Notify'")
    bus.add_message_filter(notifications)

    mainloop = glib.MainLoop()
    mainloop.run()
    print("mainloop done.")

    mk_notifications.stop()
