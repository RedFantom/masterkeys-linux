/**
 * Author: RedFantom
 * License: GNU GPLv3
 * Copyright (c) 2018-2019 RedFantom
 *
 * @file libmk.h
 * @author RedFantom
 * @date 2018-2019
 * @brief Header file of libmk
 *
 * Contains all the enums, macro and function definitions for libmk
*/
#include "libusb.h"
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/** Library settings */
#define LIBMK_USB_DESCR_LEN 255
#define LIBMK_PACKET_SIZE 64
#define LIBMK_IFACE_NUM 1     // Interface number of the HID device
#define LIBMK_PACKET_TIMEOUT 50
#define LIBMK_EP_IN 0x03
#define LIBMK_EP_OUT 0x04

#define LIBMK_MAX_ROWS 7
#define LIBMK_MAX_COLS 24
#define LIBMK_ALL_LED_PCK_NUM 8
#define LIBMK_ALL_LED_PER_PCK 16

/// @brief Error codes used within libmk
typedef enum LibMK_Result {
    LIBMK_SUCCESS = 0, ///< The one and only success code
    LIBMK_ERR_INVALID_DEV = 1, ///< Invalid device specified
    LIBMK_ERR_DEV_NOT_CONNECTED = 2, ///< Device specified not connected
    LIBMK_ERR_DEV_NOT_SET = 3, ///< Device has not been set
    LIBMK_ERR_UNKNOWN_LAYOUT = 14, ///< Device has unknown layout
    LIBMK_ERR_DEV_NOT_CLOSED = 15, ///< Device access not closed
    LIBMK_ERR_DEV_RESET_FAILED = 16, ///< Device (libusb) reset failed
    LIBMK_ERR_IFACE_CLAIM_FAILED = 4, ///< Failed to claim libusb interface
    LIBMK_ERR_IFACE_RELEASE_FAILED = 5, ///< Failed to release libusb interface
    LIBMK_ERR_DEV_CLOSE_FAILED = 6, ///< Failed to close libusb device
    LIBMK_ERR_DEV_OPEN_FAILED = 7, ///< Failed to open libusb device
    LIBMK_ERR_KERNEL_DRIVER = 8, ///< Failed to unload kernel driver
    LIBMK_ERR_DEV_LIST = 9, ///< Failed to retrieve libusb device list
    LIBMK_ERR_TRANSFER = 10, ///< Failed to transfer data to or from device
    LIBMK_ERR_DESCR = 11, ///< Failed to get libusb device descriptor
    LIBMK_ERR_PROTOCOL = 13, ///< Keyboard interaction protocol error
} LibMK_Result;


/// @brief LED Effect Types
typedef enum LibMK_Effect {
    LIBMK_EFF_FULL = 0,  ///< All LEDs in a single color
    LIBMK_EFF_BREATH = 1,  ///< All LEDs single color turning slowly on and off
    LIBMK_EFF_BREATH_CYCLE = 2,  ///< All LEDs cycling through different colors
    LIBMK_EFF_SINGLE = 3,  ///< Keystrokes highlighted with fading light
    LIBMK_EFF_WAVE = 4,  ///< Color wave over all keys
    LIBMK_EFF_RIPPLE = 5,  ///< Rippling effect from keystroke
    LIBMK_EFF_CROSS = 6,  ///< Fading cross-effect from keystroke
    LIBMK_EFF_RAIN = 7,  ///< Diagonal streaks of light
    LIBMK_EFF_STAR = 8,  ///< Fading dots in a random pattern
    LIBMK_EFF_SNAKE = 9,  ///< Snake game
    LIBMK_EFF_CUSTOM = 10,  ///< Custom LED layout
    LIBMK_EFF_OFF = 0xFE, ///< LEDs off
    
    // MasterMouse effects
    LIBMK_EFF_SPECTRUM = 11, ///< Not used
    LIBMK_EFF_RAPID_FIRE = 12 ///< Not used
} LibMK_Effect;


/// @brief Supported control modes
typedef enum LibMK_ControlMode {
    LIBMK_FIRMWARE_CTRL = 0x00, ///< Default state, no interaction
    LIBMK_EFFECT_CTRL = 0x01, ///< Software controls lighting effects
    LIBMK_CUSTOM_CTRL = 0x02, ///< Software controls individual LEDs
    LIBMK_PROFILE_CTRL = 0x03, ///< Software controls profiles
} LibMK_ControlMode;


/// @brief Supported keyboard layouts
typedef enum LibMK_Layout {
    LIBMK_LAYOUT_UNKNOWN = 0,
    LIBMK_LAYOUT_ANSI = 1,
    LIBMK_LAYOUT_ISO = 2,
    LIBMK_LAYOUT_JP = 3 ///< Currently not supported
} LibMK_Layout;


/// @brief Firmware details (version) and layout (speculative)
typedef struct LibMK_Firmware {
    unsigned char major;
    unsigned char minor;
    unsigned char patch;
    unsigned char layout;
    char string[6];
} LibMK_Firmware;


/// @brief Supported keyboard sizes
typedef enum LibMK_Size {
    LIBMK_L = 0,
    LIBMK_M = 1,
    LIBMK_S = 2
} LibMK_Size;


/// @brief Supported keyboard models
typedef enum LibMK_Model {
    DEV_RGB_L = 0,
    DEV_RGB_M = 5,
    DEV_RGB_S = 1,

    DEV_WHITE_L = 2,
    DEV_WHITE_M = 3,
    DEV_WHITE_S = 7,

    DEV_NOT_SET = -1, ///< Device not set globally
    DEV_ANY = -2, ///< Any supported device
    DEV_UNKNOWN = -3, ///< Unrecognized device
} LibMK_Model;


/** @brief Struct describing a supported USB device
 *
 * This struct may be used as a linked list. Holds information required
 * for the identification of the keyboard on the machine.
 */
typedef struct LibMK_Device {
    char* iManufacturer; ///< Manufacturer string
    char* iProduct;  ///< Product string
    int bVendor; ///< USB Vendor ID number
    int bDevice; ///< USB Device ID number
    LibMK_Model model; ///< Model number
    struct LibMK_Device* next; ///< Linked list attribute
    libusb_device* device; ///< libusb_device struct instance
} LibMK_Device;


typedef struct LibMK_Handle {
    /** Struct describing an opened device */
    LibMK_Model model;
    int bVendor;
    int bDevice;
    int layout;
    int size;
    libusb_device_handle* handle;
    bool open;
} LibMK_Handle;


typedef struct LibMK_Effect_Details {
    LibMK_Effect effect;
    unsigned char speed;
    unsigned char direction;
    unsigned char amount;
    unsigned char foreground[3];
    unsigned char background[3];
} LibMK_Effect_Details;


/** Library management and initialization */
bool libmk_init();
int libmk_exit();

/** Device management */
int libmk_detect_devices(LibMK_Model** model_list);
LibMK_Device* libmk_open_device(libusb_device* device);
LibMK_Device* libmk_create_device(
    LibMK_Model model, libusb_device* device,
    char* iManufacturer, char* iProduct,
    int bVendor, int bDevice);
void libmk_free_device(LibMK_Device* device);
LibMK_Device* libmk_append_device(LibMK_Device* first, LibMK_Device* device);
LibMK_Model libmk_ident_model(char* product);


/** Handle management */
int libmk_create_handle(LibMK_Handle** handle, LibMK_Device* device);
int libmk_free_handle(LibMK_Handle* handle);


/** Device control */
int libmk_set_device(LibMK_Model model, LibMK_Handle** handle);
int libmk_enable_control(LibMK_Handle* handle);
int libmk_disable_control(LibMK_Handle* handle);
int libmk_claim_interface(LibMK_Handle* handle);
int libmk_send_control_packet(LibMK_Handle* handle);
int libmk_reset(LibMK_Handle* handle);
int libmk_get_device_ident(LibMK_Handle* handle);
int libmk_get_firmware_version(LibMK_Handle* handle, LibMK_Firmware** fw);


/** Communication */
int libmk_send_packet(LibMK_Handle* handle, unsigned char* packet);
unsigned char* libmk_build_packet(unsigned char predef, ...);


/** LED Control */
int libmk_set_effect(LibMK_Handle* handle, LibMK_Effect effect);
int libmk_set_effect_details(
    LibMK_Handle* handle, LibMK_Effect_Details* effect);
int libmk_set_full_color(
    LibMK_Handle* handle, unsigned char r, unsigned char g, unsigned char b);
int libmk_set_all_led_color(
    LibMK_Handle* handle,
    unsigned char* colors);
int libmk_set_single_led(
    LibMK_Handle* handle, unsigned char row, unsigned char col,
    unsigned char r, unsigned char g, unsigned char b);
int libmk_get_offset(
    unsigned char* offset, LibMK_Handle* handle,
    unsigned char row, unsigned char col);


/** Debugging purposes */
void libmk_print_packet(unsigned char* packet);
