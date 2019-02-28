"""
Author: RedFantom
License: GNU GPLv3
Copyright (c) 2018-2019 RedFantom
"""
try:
    from . import masterkeys as _mk
except ImportError:
    import warnings
    warnings.warn("Failed to import masterkeys C library", ImportWarning)
try:
    from typing import Dict, List, Tuple
except ImportError:  # PyCharm typing
    pass


MAX_ROWS = 7
MAX_COLS = 24


class ResultCode:
    # Success codes
    SUCCESS = 0

    # Device Errors
    ERR_INVALID_DEV = -1
    ERR_DEV_NOT_CONNECTED = -2
    ERR_DEV_NOT_SET = -3
    ERR_UNKNOWN_LAYOUT = -14
    ERR_DEV_NOT_CLOSED = -15
    ERR_DEV_RESET_FAILED = -16

    # Interface Errors
    ERR_IFACE_CLAIM_FAILED = -4
    ERR_IFACE_RELEASE_FAILED = -5
    ERR_DEV_CLOSE_FAILED = -6
    ERR_DEV_OPEN_FAILED = -7

    # Kernel Driver Errors
    ERR_KERNEL_DRIVER = -8

    # Communication Errors
    ERR_DEV_LIST = -9
    ERR_TRANSFER = -10
    ERR_DESCR = -11

    # Protocol Errors
    ERR_PROTOCOL = -13


class Effect:
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


class Model:
    MODEL_RGB_L = 0
    MODEL_RGB_M = 5
    MODEL_RGB_S = 1

    MODEL_WHITE_L = 2
    MODEL_WHITE_M = 3
    MODEL_WHITE_S = 7

    MODEL_NOT_SET = -1
    MODEL_ANY = -2
    MODEL_UNKNOWN = -3


class ControlMode:
    FIRMWARE_CTRL = 0x00
    EFFECT_CTRL = 0x01
    CUSTOM_CTRL = 0x02
    PROFILE_CTRL = 0x03


MODEL_STRINGS = {
    0: "MasterKeys Pro L RGB",
    5: "MasterKeys Pro M RGB",
    1: "MasterKeys Pro S RGB",
    2: "MasterKeys Pro L White",
    3: "MasterKeys Pro M White",
    7: "MasterKeys Pro S White",
    -1: "Model not set",
    -2: "Any supported model",
    -3: "Unknown model"
}


def detect_devices():
    # type: () -> Tuple[int, ...]
    """
    Detect supported connected devices and return a tuple of models
    
    :return: Tuple of integers (:class:`.Model`)
    :rtype: Tuple[int, ...]
    :raises: ``RuntimeError`` upon internal Python error
    """
    return _mk.detect_devices()


def set_device(model):
    # type: (int) -> int
    """
    Set the device to be controlled by the library to the specified model

    :param model: Model to be controlled by the library. Only one device
        is supported in the Python library. Only the first found device
        of the specified model is controlled.
    :type model: int
    :return: Result code (:class:`.ResultCode`)
    :rtype: int
    :raises: ``TypeError`` upon invalid argument type
    """
    return _mk.set_device(model)


def enable_control():
    # type: () -> int
    """
    Enable control on the device that has been set

    :return: Result code (:class:`.ResultCode`)
    :rtype: int
    """
    return _mk.enable_control()


def disable_control():
    # type: () -> int
    """
    Disable control on the device that has been set and is controlled

    :return: Result code (:class:`.ResultCode`)
    :rtype: int
    """
    return _mk.disable_control()


def set_effect(effect):
    # type: (int) -> int
    """
    Set the effect to be active on the controlled keyboard

    :param effect: Effect number to set to be active (:class:`.Effect`)
    :type effect: int
    :return: Result code (:class:`.ResultCode`)
    :rtype: int
    :raises: ``TypeError`` upon invalid argument type
    """
    return _mk.set_effect(effect)


def set_all_led_color(layout):
    # type: (List[List[Tuple[int, int, int], ...], ...]) -> int
    """
    Set the color of all LEDs on the keyboard individually

    :param layout: List of lists of color tuples such as created by
        build_layout_list()
    :type layout: List[List[Tuple[int, int, int], ...], ...]
    :return: Result code (:class:`.ResultCode`)
    :rtype: int
    :raises: ``ValueError`` if the wrong amount of elements is in the
        list
    :raises: ``TypeError`` if invalid argument type (any element)
    """
    return _mk.set_all_led_color(layout)


def set_full_led_color(r, g, b):
    # type: (int, int, int) -> int
    """
    Set the color of all the LEDs on the keyboard to a single color

    :param r: red color byte
    :type r: int
    :param g: green color byte
    :type g: int
    :param b: blue color byte
    :type b: int
    :return: Result code (:class:`.ResultCode`)
    :rtype: int
    """
    return _mk.set_full_led_color(r, g, b)


def set_ind_led_color(row, col, r, g, b):
    # type: (int, int, int, int, int) -> int
    """
    Set the color of a single, individual key on the keyboard

    :param row: zero-indexed row index < MAX_ROWS
    :type row: int
    :param col: zero-indexed column index < MAX_COLS
    :type col: int
    :param r: red color byte
    :type r: int
    :param g: green color byte
    :type g: int
    :param b: blue color byte
    :type b: int
    :return: Result code (:class:`.ResultCode`)
    :rtype: int
    :raises: ``TypeError`` upon invalid argument type
    """
    return _mk.set_ind_led_color(row, col, r, g, b)


def get_device_ident():
    # type: () -> int
    """
    Return the bDevice USB descriptor value for the controlled keyboard

    :return: bDevice USB descriptor value or result code (<0)
    :rtype: int
    """
    return _mk.get_device_ident()


def set_all_led_color_dict(keys):
    # type: (Dict[Tuple[int, int], Tuple[int, int, int]]) -> int
    """
    Set the color of all LEDs on the controlled device with a dictionary

    The keys should be specified in a dictionary of the format
    {(row, col): (r, g, b)}

    :param keys: Dictionary containing key color data
    :type keys: Dict[Tuple[int, int], Tuple[int, int, int]]
    :return: Result code (:class:`.ResultCode`)
    :rtype: int
    """
    layout = build_layout_list()
    for (row, col), (r, g, b) in keys.items():
        layout[row][col] = (r, g, b)
    return _mk.set_all_led_color(layout)


def build_layout_list():
    # type: () -> List[List[Tuple[int, int, int], ...], ...]
    """
    Return a list of the right proportions for full LED layout

    :return: Empty layout list
    :rtype: List[List[Tuple[int, int, int], ...], ...]
    """
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
    # type: (int, int, int, int, Tuple[int, int, int], Tuple[int, int, int]) -> int
    """
    Set an effect with custom parameters

    For more details about the parameters, please see the libmk
    documentation.

    :param effect: Effect number (:class:`.Effect`)
    :type effect: int
    :param direction: Direction of the animation of the effect
    :type direction: int
    :param speed: Speed of the animation of the effect
    :type speed: int
    :param amount: Amount/intensity of the effect
    :type amount: int
    :param foreground: Foreground color of the effect
    :type foreground: Tuple[int, int, int]
    :param background: Background color of the effect
    :type background: Tuple[int, int, int]
    :return: Result code (:class:`.ResultCode`)
    :rtype: int
    """
    return _mk.set_effect_details(
        effect, direction, speed, amount, foreground, background)


def get_active_profile():
    # type: () -> int
    """
    Return the number of the profile active on the keyboard

    :return: Result code (:class:`.ResultCode`) or profile number
    :rtype: int
    """
    return _mk.get_active_profile()


def set_active_profile(profile):
    # type: (int) -> int
    """
    Activate a profile on the keyboard

    :param profile: Number of profile to activate
    :type profile: int
    :return: Result code (:class:`.ResultCode`)
    :rtype: int
    """
    return _mk.set_active_profile(profile)


def save_profile():
    # type: () -> int
    """
    Save the changes made to the lighting to the active profile

    :return: Result code (:class:`.ResultCode`)
    :rtype: int
    """
    return _mk.save_profile()


def set_control_mode(mode):
    # type: (int) -> int
    """
    Change the control mode of the keyboard manually

    :param mode: New control mode to set (:class:`.ControlMode`)
    :type mode: int  
    :return: Result code (:class:`.ResultCode`)
    :rtype: int
    """
    return _mk.set_control_mode(mode)

