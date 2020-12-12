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

/// @brief Maximum number of rows supported on any device
#define LIBMK_MAX_ROWS 7
/// @brief Maximum number of columns supported on any device
#define LIBMK_MAX_COLS 24
#define LIBMK_ALL_LED_PCK_NUM 8
#define LIBMK_ALL_LED_PER_PCK 16

/// @brief Error codes used within libmk
typedef enum LibMK_Result {
    LIBMK_SUCCESS = 0, ///< The one and only success code
    LIBMK_ERR_INVALID_DEV = -1, ///< Invalid device specified
    LIBMK_ERR_DEV_NOT_CONNECTED = -2, ///< Device specified not connected
    LIBMK_ERR_DEV_NOT_SET = -3, ///< Device has not been set
    LIBMK_ERR_UNKNOWN_LAYOUT = -14, ///< Device has unknown layout
    LIBMK_ERR_DEV_NOT_CLOSED = -15, ///< Device access not closed
    LIBMK_ERR_DEV_RESET_FAILED = -16, ///< Device (libusb) reset failed
    LIBMK_ERR_IFACE_CLAIM_FAILED = -4, ///< Failed to claim libusb interface
    LIBMK_ERR_IFACE_RELEASE_FAILED = -5, ///< Failed to release libusb interface
    LIBMK_ERR_DEV_CLOSE_FAILED = -6, ///< Failed to close libusb device
    LIBMK_ERR_DEV_OPEN_FAILED = -7, ///< Failed to open libusb device
    LIBMK_ERR_KERNEL_DRIVER = -8, ///< Failed to unload kernel driver
    LIBMK_ERR_DEV_LIST = -9, ///< Failed to retrieve libusb device list
    LIBMK_ERR_TRANSFER = -10, ///< Failed to transfer data to or from device
    LIBMK_ERR_DESCR = -11, ///< Failed to get libusb device descriptor
    LIBMK_ERR_PROTOCOL = -13, ///< Keyboard interaction protocol error
    LIBMK_ERR_INVALID_ARG = -14, ///< Invalid arguments passed by caller
    LIBMK_ERR_STILL_ACTIVE = -15, ///< Controller is still active
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


/** @brief Array of strings representing the supported models */
extern const char* LIBMK_MODEL_STRINGS[];

/** @brief Struct describing an opened supported device
 *
 * Result of libmk_set_device(LibMK_Model, LibMK_Handle**). Contains all
 * the required data to control a keyboard device. Contains a
 * libusb_device_handle* to allow interaction with the keyboard
 * endpoints by the library. Multiple of these may be availble in your
 * program, but no multiple handles for a single device are allowed.
 */
typedef struct LibMK_Handle {
    LibMK_Model model; ///< Model of the keyboard this handle controls
    int bVendor; ///< USB bVendor ID number
    int bDevice; ///< USB bDevice ID number
    LibMK_Layout layout; ///< Layout of this device
    LibMK_Size size; ///< Size of this device
    libusb_device_handle* handle; ///< libusb_device_handle required for
                                  ///< reading from and writing to the
                                  ///< device endpoints
    bool open; ///< Current state of the handle. If closed, the handle
               ///< is no longer valid. Handles may not be re-opened.
} LibMK_Handle;


/** @brief Struct describing an effect with custom settings
 *
 * Apart from the default settings that are applied when an effect is
 * applied using libmk_set_effect, custom settings may be applied to
 * and effect. Each effect implements these settings differently.
 *
 * For example: by using a background color of red and a foreground
 * color orange with the effect LIBMK_EFF_STAR, a star effect is applied
 * of fading orange-lit keys. All keys that are not 'stars' are red in
 * color. The amount attribute changes the amount of 'stars' shown and
 * the speed attribute the speed with which they fade in and out.
 *
 * Not all attributes are supported by all effects. Some will have no
 * influence at all.
 */
// TODO: Document all the parameters for all the effects
typedef struct LibMK_Effect_Details {
    LibMK_Effect effect;
    unsigned char speed; ///< Running speed of the effect
    unsigned char direction; ///< Direction of the effect
    unsigned char amount; ///< Intensity modifier of the effect. Not
                          ///< supported by all effects in the same way.
    unsigned char foreground[3]; ///< Foreground color of the effect
    unsigned char background[3]; ///< Background color of the effect
} LibMK_Effect_Details;

/** @brief Initialize the library and its dependencies to a usable state
 *
 * Initializes a default libusb context for use throughout the library.
 * If a call to this function is omitted, segmentation faults will be
 * the result.
*/
bool libmk_init();

/** @brief Clean-up the library resources
 *
 * Frees up the memory used by the libusb context used throughout the
 * library. Support for re-initialization after this function is called
 * is not guaranteed. Use only at the end of the program.
 */
int libmk_exit();

/** @brief Search for devices and put the found models in an array
 *
 * @param model_list: Pointer to an array of LibMK_Model enums. Required
 *    memory for storage is allocated by the function and must be freed
 *    by the caller.
 * @returns The number of found devices, or a LibMK_Result error code.
 *
 * Perform a search for devices using libusb and store all the found
 * supported devices in an allocated array of LibMK_Model.
 */
int libmk_detect_devices(LibMK_Model** model_list);

/** @brief Internal function. Loads the details of a device
 *
 * @param device: libusb device descriptor to load details for
 * @returns pointer to LibMK_Device instance if successful, NULL
 *    otherwise
 *
 * Loads the details of a USB device into a LibMK_Device struct instance.
 * The details are used by libmk_detect_devices to determine whether a
 * device is supported.
 */
LibMK_Device* libmk_open_device(libusb_device* device);

/** @brief Internal function. Allocate and fill LibMK_Device struct */
LibMK_Device* libmk_create_device(
    LibMK_Model model, libusb_device* device,
    char* iManufacturer, char* iProduct,
    int bVendor, int bDevice);

/** @brief Internal function. Free memory of allocated LibMK_Device */
void libmk_free_device(LibMK_Device* device);

/** @brief Internal function. Build linked list with LibMK_Device */
LibMK_Device* libmk_append_device(LibMK_Device* first, LibMK_Device* device);

/** @brief Identify a model based on its USB descriptor product string */
LibMK_Model libmk_ident_model(char* product);

/** @brief Internal function. Allocate and fill LibMK_Handle struct */
int libmk_create_handle(LibMK_Handle** handle, LibMK_Device* device);

/** @brief Internal function. Free memory of allocated LibMK_Handle */
int libmk_free_handle(LibMK_Handle* handle);

/** @brief Initialize a device within the library
 *
 * @param model: Model to initialize. The model must be connected, else
 *    LIBMK_ERR_DEV_NOT_CONNECTED is returned.
 * @param handle: Pointer to pointer of struct LibMK_Handle. The funtion
 *    allocates a LibMK_Handle struct and stores a pointer to it here.
 * @returns LibMK_Result code, NULL or valid pointer in *handle.
 *
 * If NULL is passed for handle the function will set the global
 * LibMK_Handle to the device specified. The function chooses the first
 * available device of the specified model to assign to the handle.
 */
int libmk_set_device(LibMK_Model model, LibMK_Handle** handle);

/** @brief Initialize the keyboard for control and send control packet
 *
 * @param handle: LibMK_Handle* for the device to control. If NULL, the
 *    global handle is used.
 * @returns LibMK_Result result code
 *
 * Must be called in order to be able to control the keyboard. Claims
 * the LED interface on the keyboard USB controller with the appropriate
 * endpoints for control.
 */
int libmk_enable_control(LibMK_Handle* handle);

/** @brief Release the keyboard from control
 *
 * @param handle: LibMK_Handle* for the device to release. If NULL, the
 *    global handle is used.
 * @returns LibMK_Result result code
 *
 * Must be called when the user is done controlling the keyboard.
 * Support for re-enabling of control on the same LibMK_Handle is not
 * guaranteed.
 */
int libmk_disable_control(LibMK_Handle* handle);

/** @brief Internal function. Claims USB LED interface on device */
int libmk_claim_interface(LibMK_Handle* handle);

/** @brief Sends packet to put the keyboard in LIBMK_EFFECT_CTRL
 *
 * @param handle: LibMK_Handle* for the device to send the packet to. If
 *    NULL, the global handle is used.
 * @returns LibMK_Result result code
 */
int libmk_send_control_packet(LibMK_Handle* handle);

/** @brief Internal function. Reset the libusb interface when releasing */
int libmk_reset(LibMK_Handle* handle);

/** @brief Internal function. Return the bDevice USB descriptor property */
int libmk_get_device_ident(LibMK_Handle* handle);

/** @brief Retrieve details on the firmware version of the device
 *
 * @param handle: Handle to the device to retrieve firmware version of.
 *    If NULL, the global handle is used.
 * @param fw: A LibMK_Firmware* is provided here if the firmware details
 *    are retrieved successfully.
 * @returns LibMK_Result result code, NULL or LibMK_Firmware* in fw
 */
int libmk_get_firmware_version(LibMK_Handle* handle, LibMK_Firmware** fw);


int libmk_set_control_mode(LibMK_Handle* handle, LibMK_ControlMode mode);

/** @brief Send a single packet and verify the response
 *
 * @param handle: LibMK_Handle for the device to send the packet to
 * @param packet: Array of bytes (unsigned char) to send of size
 *    LIBMK_PACKET_SIZE
 * @returns LibMK_Result result code
 *
 * Sends a single packet of data to the keyboard of size
 * LIBMK_PACKET_SIZE (will segfault if not properly sized!) and
 * verifies the response of the keyboard by checking the header
 * of the response packet. Frees the memory of the provided packet
 * pointer.
 */
int libmk_send_packet(LibMK_Handle* handle, unsigned char* packet);


/** @brief Exchange a single packet with the keyboard
 *
 * @param handle: LibMK_Handle for the device to exchange data with
 * @param packet: Array of bytes (unsigned char) to send of size
 *    LIBMK_PACKET_SIZE and to put response in
 * @returns LibMK_Result result code, response in packet
 *
 * Send a single packet to the keyboard and then store the response
 * in the packet array. Does not free the packet memory or verify
 * the response as an error response.
 */
int libmk_exch_packet(LibMK_Handle* handle, unsigned char* packet);

/** @brief Set effect to be active on keyboard
 *
 * @param handle: LibMK_Handle for device to set effect on. If NULL uses
 *    global device handle
 * @param effect: LibMK_Effect specifier of effect to activate
 * @returns LibMK_Result result code
 */
int libmk_set_effect(LibMK_Handle* handle, LibMK_Effect effect);

/** @brief Set effect to be active on keyboard with parameters
 *
 * @param handle: LibMK_Handle for device to set effect on. If NULL uses
 *    global device handle.
 * @param effect: LibMK_Effect_Details instance with all parameters set
 * @returns LibMK_Result result code
 */
int libmk_set_effect_details(LibMK_Handle* handle, LibMK_Effect_Details* effect);

/** @brief Set color of all the LEDs on the keyboard to a single color
 *
 * @param handle: LibMK_Handle for device to set color off. If NULL uses
 *    global device handle.
 * @param r: color byte red
 * @param g: color byte green
 * @param b: color byte blue
 * @returns LibMK_Result result code
 */
int libmk_set_full_color(
    LibMK_Handle* handle, unsigned char r, unsigned char g, unsigned char b);

/** @brief Set the color of all the LEDs on the keyboard individually
 *
 * @param handle: LibMK_Handle for device to set the key colors on. If
 *    NULL uses the global device handle.
 * @param colors: Pointer to array of unsigned char of
 *    [LIBMK_MAX_ROWS][LIBMK_MAX_COLS][3] size.
 * @returns LibMK_Result result code
 */
int libmk_set_all_led_color(LibMK_Handle* handle, unsigned char* colors);

/** @brief Set the color of a single LED on the keyboard
 *
 * @param handle: LibMK_Handle for device to set the color of the key
 *    on. If NULL uses the global device handle.
 * @param row: Zero-indexed row index
 * @param col: Zero-indexed column index
 * @param r: color byte red
 * @param g: color byte green
 * @param b: color byte blue
 * @returns LibMK_Result result code
 */
int libmk_set_single_led(
    LibMK_Handle* handle, unsigned char row, unsigned char col,
    unsigned char r, unsigned char g, unsigned char b);

/** @brief Retrieve the addressing offset of a specific key
 *
 * @param offset: Pointer to unsigned char to store offset in
 * @param handle: LibMK_Handle for the device to find the offset for. Is
 *    required in order to determine the layout of the device.
 * @param row: Zero-indexed row index
 * @param col: Zero-indexed column index
 * @returns LibMK_Result result code
 */
int libmk_get_offset(
    unsigned char* offset, LibMK_Handle* handle,
    unsigned char row, unsigned char col);

/** @brief Set the profile active on the device
 *
 * @param handle: LibMK_Handle for the device to set the profile on. If
 *    NULL the global device handle is used.
 * @param profile: Number of the profile to activate.
 * @returns LibMK_Result result code
 *
 * The MasterKeys series of devices supports four individual lighting
 * profiles to which settings can be saved. A profile is activated by
 * changing the control mode and then activating the new profile. Any
 * changes applied to the lighting of the keyboard are lost when
 * changing the control mode to profile control.
 */
int libmk_set_active_profile(LibMK_Handle* handle, char profile);

/** @brief Retrieve the number of the active profile on the device
 *
 * @param handle: LibMK_Handle for the device to get the number of the
 *    active profile from. If NULL the global device handle is used.
 * @param profile: Pointer to the unsigned char to store the number of
 *    the profile in.
 * @returns LibMK_Result result code
 */
int libmk_get_active_profile(LibMK_Handle* handle, char* profile);

/** @brief Save the current lighting settings to the active profile
 *
 * @param handle: LibMK_Handle for the device to save the changes to. If
 *    NULL the global device handle is used.
 * @returns LibMK_Result result code
 *
 * Saves the lighting settings to the profile that is returned by
 * libmk_get_active_profile. The changes persist after releasing control
 * of the keyboard, and even after power-cycling the keyboard. The
 * previous lighting configuration in the profile is deleted and
 * non-recoverable.
 */
int libmk_save_profile(LibMK_Handle* handle);

/** @brief Send a packet specifying whether to expect a response
 *
 * @param handle: LibMK_Handle of the device to send the packet to. If
 *    NULL the global device handle is used.
 * @param packet: Pointer to array of packet to send.
 * @param r: Whether to expect a response. If ``false``, the function
 *    still attempts to read a response to make sure that the buffer is
 *    empty. If ``true``, performs protocol error checks and yields an
 *    error if no response was received.
 * @returns LibMK_Result result code
 */
int libmk_send_recv_packet(LibMK_Handle* handle, unsigned char* packet, bool r);

/** @brief Build a new packet of data that can be sent to keyboard
 *
 * @param predef: Amount of bytes given in the variable arguments
 * @param ...: Bytes to from index zero of the packet. The amount of
 *    bytes given must be equal to the amount specified by ``predef``,
 *    otherwise this function will segfault.
 * @returns Pointer to the allocated packet with the set bytes. NULL if
 *    no memory could be allocated.
 */
unsigned char* libmk_build_packet(unsigned char predef, ...);

/** Debugging purposes */
void libmk_print_packet(unsigned char* packet, char* label);
