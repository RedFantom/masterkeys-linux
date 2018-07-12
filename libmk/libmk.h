/**
 * Author: RedFantom
 * License: GNU GPLv3
 * Copyright (c) 2018 RedFantom
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


typedef enum LibMK_Result {
    /** Describes error codes that are used in libmk */
    // Success codes
    LIBMK_SUCCESS = 0,
    // Device errors
    LIBMK_ERR_INVALID_DEV = 1,
    LIBMK_ERR_DEV_NOT_CONNECTED = 2,
    LIBMK_ERR_DEV_NOT_SET = 3,
    LIBMK_ERR_UNKNOWN_LAYOUT = 14,
    LIBMK_ERR_DEV_NOT_CLOSED = 15,
    LIBMK_ERR_DEV_RESET_FAILED = 16,
    // Interface errors
    LIBMK_ERR_IFACE_CLAIM_FAILED = 4,
    LIBMK_ERR_IFACE_RELEASE_FAILED = 5,
    LIBMK_ERR_DEV_CLOSE_FAILED = 6,
    LIBMK_ERR_DEV_OPEN_FAILED = 7,
    // Kernel driver errors
    LIBMK_ERR_KERNEL_DRIVER = 8,
    // Communication errors
    LIBMK_ERR_DEV_LIST = 9,
    LIBMK_ERR_TRANSFER = 10,
    LIBMK_ERR_DESCR = 11,
    LIBMK_ERR_SEND = 12,
    // Protocol errors
    LIBMK_ERR_PROTOCOL = 13,
} LibMK_Result;


typedef enum LibMK_Effect {
    /** Describes LED effect types
     *
     *  All these effects are actually stored on the microcontroller
     *  in the keyboard. Applying the effect has the effect of also
     *  storing it in a profile because of this limitation.
     */
    LIBMK_EFF_FULL = 0,  // All LEDs in a single color
    LIBMK_EFF_BREATH = 1,  // All LEDs single color turning slowly on and off
    LIBMK_EFF_BREATH_CYCLE = 2,  // All LEDs cycling through different colors
    LIBMK_EFF_SINGLE = 3,  // Keystrokes highlighted with fading light
    LIBMK_EFF_WAVE = 4,  // Color wave
    LIBMK_EFF_RIPPLE = 5,  // Rippling effect from keystroke
    LIBMK_EFF_CROSS = 6,  // Fading cross-effect from keystroke
    LIBMK_EFF_RAIN = 7,  // Diagonal streaks of light
    LIBMK_EFF_STAR = 8,  // Fading dots in a random pattern
    LIBMK_EFF_SNAKE = 9,  // Snake game
    LIBMK_EFF_CUSTOM = 10,  // Custom LED layout
    LIBMK_EFF_OFF = 0xFE,
    
    // MasterMouse effects
    LIBMK_EFF_SPECTRUM = 11,
    LIBMK_EFF_RAPID_FIRE = 12
} LibMK_Effect;


typedef enum LibMK_Model {
    /** Describes models available for use with this library
     *
     * The device indices are the same as the ones used in the official
     * Cooler Master (Windows-only) SDK. Not all these devices are
     * supported. For white devices, the
     */
    DEV_RGB_L = 0,
    DEV_RGB_M = 5,
    DEV_RGB_S = 1,

    DEV_WHITE_L = 2,
    DEV_WHITE_M = 3,
    DEV_WHITE_S = 7,

    DEV_NOT_SET = -1,
    DEV_ANY = -2,
    DEV_UNKNOWN = -3,
} LibMK_Model;


typedef struct LibMK_Device {
    /** Struct describing a supported USB device
     *
     * This struct supports usage as a linked list with the 'next'
     * attribute. Holds information required for identifying the
     * keyboard layout and other attributes.
     */
    char* iManufacturer;  // Manufacturer string
    char* iProduct;  // Product string
    int bVendor;
    int bDevice;
    LibMK_Model model;
    struct LibMK_Device* next;
    libusb_device* device;
} LibMK_Device;


typedef struct LibMK_Handle {
    /** Struct describing an opened device */
    LibMK_Model model;
    int bVendor;
    int bDevice;
    int layout;
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
unsigned char libmk_get_layout(int bProduct);


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
