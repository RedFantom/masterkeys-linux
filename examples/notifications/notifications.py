"""
Author: RedFantom
License: GNU GPLv3
Copyright (c) 2019 RedFantom
"""
import mk_notifications
import time
from gi.repository import GLib as glib
import dbus
from dbus.mainloop.glib import DBusGMainLoop


def notifications(bus, message):
    print([arg for arg in message.get_arg_list()])


if __name__ == '__main__':
    r = mk_notifications.init(2, 25, 700, 60, 1, 20.0, 2, 1.0)
    if r is None:
        raise RuntimeError()
    mk_notifications.start()

    DBusGMainLoop(set_as_default=True)
    bus = dbus.SystemBus()
    bus.add_match_string_non_blocking("eavesdrop=true, interface='org.freedesktop.Notifications', member='Notify'")
    bus.add_message_filter(notifications)

    mainloop = glib.MainLoop()
    mainloop.run()
    print("mainloop done.")

    mk_notifications.stop()
