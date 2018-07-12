"""
Author: RedFantom
License: GNU GPLv3
Copyright (c) 2018 RedFantom


void __import__()
    Function executed upon import of module. Allocates resources for
    dependencies and registers clean-up function for Python interpreter
    clean exit.
    :raises: RuntimeError if initialization fails

tuple detect_devices()
    Returns a tuple of connected models. Can contain duplicate models,
    which is not supported in the rest of the library. Requires root
    privileges on most platforms.
    :returns: tuple of Model enums
    :raises: RuntimeError upon internal Python error

int set_device(model)
    Set the device to use with the library. Only a single device is
    supported. If multiple devices of the same model are connected, the
    first encountered is set to control.
    :param model: Model number of device to set
    :return: result code (SUCCESS or ERR*)
    :raises: TypeError upon invalid argument type

int enable_control()
    Send the control packet to the keyboard set to control.
    :return: result code (SUCCESS or ERR*)

int disable_control()
    Send the packet to release control to the keyboard set to control.
    :return: result code (SUCCESS or ERR*)

int set_effect(effect)
    Set one of the built-in LED effects on the keyboard.
    :param effect: Effect number (EFF*)
    :return: result code (SUCCESS or ERR*)
    :raises: TypeError upon invalid argument type

int set_full_led_color(r, g, b)
    Set the color of all the LEDs on the keyboard to a single color.
    :param r: Red color value, 0-255 (two symbol HEX)
    :param g: Green color value
    :param b: Blue color value
    :return: result code (SUCCESS or ERR*)
    :raises: TypeError upon invalid argument type or count

int set_all_led_color(layout)
    Set the color of all the LEDs on the keyboard from a list of lists
    :param layout: List of lists of tuples ([row][col][index])
    :return: result code (SUCCESS or ERR*)
    :raises: TypeError if invalid type in the argument
    :raises: ValueError if invalid amount of elements in the argument

int set_ind_led_color(row, col, r, g, b)
    Set the color of a single LED on the keyboard. Does not work in
    conjunction with the set_full_led_color function.
    :param row, col: Coordinates of LED to set color of
    :param r, g, b: Color values
    :return: result code (SUCCESS or ERR*)
    :raises: TypeError upon invalid argument type
"""
from .masterkeys import *


MAX_ROWS = 7
MAX_COLS = 24

# Success codes
SUCCESS = 0

# Device Errors
ERR_INVALID_DEV = 1
ERR_DEV_NOT_CONNECTED = 2
ERR_DEV_NOT_SET = 3
ERR_UNKNOWN_LAYOUT = 14
ERR_DEV_NOT_CLOSED = 15
ERR_DEV_RESET_FAILED = 16

# Interface Errors
ERR_IFACE_CLAIM_FAILED = 4
ERR_IFACE_RELEASE_FAILED = 5
ERR_DEV_CLOSE_FAILED = 6
ERR_DEV_OPEN_FAILED = 7

# Kernel Driver Errors
ERR_KERNEL_DRIVER = 8

# Communication Errors
ERR_DEV_LIST = 9
ERR_TRANSFER = 10
ERR_DESCR = 11  # Descriptor
ERR_SEND = 12

# Protocol Errors
ERR_PROTOCOL = 13

# Effects
EFF_FULL_ON = 0
EFF_BREATH = 1
EFF_BREATH_CYCLE = 2
EFF_SINGLE = 3
EFF_WAVE = 4
EFF_RIPPLE = 5
EFF_CROSS = 6
EFF_RAIN = 7
EFF_STAR = 8
EFF_SNAKE = 9
EFF_REC = 10
EFF_SPECTRUM = 11
EFF_RAPID_FIRE = 12

# Models
MODEL_RGB_L = 0
MODEL_RGB_M = 5
MODEL_RGB_S = 1

MODEL_WHITE_L = 2
MODEL_WHITE_M = 3
MODEL_WHITE_S = 7

MODEL_NOT_SET = -1
MODEL_ANY = -2
MODEL_UNKNOWN = -3


def set_all_led_color_dict(keys):
    """
    Set the color of all LEDs on the controlled device with a dictionary

    The keys should be specified in a dictionary of the format
    {(row, col): (r, g, b)}
    """
    layout = build_layout_list()
    for (row, col), (r, g, b) in keys.items():
        layout[row][col] = (r, g, b)
    return set_all_led_color(layout)


def build_layout_list():
    """Return a list of the right proportions for full LED layout"""
    layout = list()
    for i in range(MAX_ROWS):
        column = list()
        for j in range(MAX_COLS):
            column.append((0x00, 0x00, 0x00))
        layout.append(column)
    return layout


def set_effect_details(effect, direction=0, speed=0x60, amount=0x00,
                       foreground=(0xFF, 0xFF, 0xFF),
                       background=(0x00, 0x00, 0x00)):
    return _set_effect_details(
        effect, direction, speed, amount, foreground, background)
