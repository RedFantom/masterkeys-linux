/** Author: RedFantom
 * License: GNU GPLv3
 * Copyright (c) 2018-2019 RedFantom
*/
#include "libmk.h"
#include "libusb.h"
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// #define LIBMK_DEBUG
// #define LIBMK_USB_DEBUG

#ifndef LIBMK_DEBUG
#define printf(fmt, ...) (0)
#endif

#define MANUFACTURER "Cooler Master Technology Inc."
#define PRODUCT "MasterKeys"
#define WHITE "White"
#define LARGE "L"
#define MEDIUM "M"
#define SMALL "S"


/** Global Variables
 *
 * Each instance of the library gets its own copy of these variables.
 * The variables are required so that if the library is loaded into a
 * different language (like Python), interaction is easier.
*/
static libusb_context* Context;
static LibMK_Handle* DeviceHandle;

typedef enum LibMK_Model LibMK_Model;
typedef enum LibMK_Result LibMK_Result;
typedef enum LibMK_Effect LibMK_Effect;
typedef struct LibMK_Device LibMK_Device;

/** Constants */
const unsigned char HEADER_SET = 0x51;            // 1001 0001
const unsigned char HEADER_GET = 0x52;            // 1001 0010

const unsigned char HEADER_FULL_COLOR = 0xc0;     // 1100 0000
const unsigned char HEADER_ERROR = 0xFF;          // 1111 1111
const unsigned char OPCODE_EFFECT = 0x28;         // 0010 1000
const unsigned char OPCODE_ALL_LED = 0xA8;        // 1010 1000
const unsigned char OPCODE_EFFECT_ARGS = 0x2c;    // 0010 1100

const unsigned int ANSI[] =
    {0x003b, 0x0000};
const unsigned int ISO[] =
    {0x0047, 0x0000};

const unsigned char LAYOUT_ANSI = 0;
const unsigned char LAYOUT_ISO = 1;

const char* LIBMK_MODEL_STRINGS[] = {
    "MasterKeys Pro L RGB",
    "MasterKeys Pro S RGB",
    "MasterKeys Pro L White",
    "MasterKeys Pro M White",
    "Unknown Model",
    "MasterKeys Pro M RGB",
    "Unknown Model",
    "MasterKeys Pro S White",
};

/** Layout matrices
 *
 * These matrices describe the internal addresses of keys within the
 * keyboard. The matrix is in [row][column] format. A value of 0xFF
 * indicates an unknown value.
*/
const unsigned char LIBMK_LAYOUT[2][3][LIBMK_MAX_ROWS][LIBMK_MAX_COLS] = {
    { // ANSI Layouts
        // Layout size L
        {{0x0b, 0x16, 0x1e, 0x19, 0x1b, 0xff, 0x07, 0x33, 0x39, 0x3e, 0xff, 0x56,
          0x57, 0x53, 0x55, 0x4f, 0x48, 0x00, 0x65, 0x6d, 0x75, 0x77, 0xff, 0xff},
         {0x0e, 0x0f, 0x17, 0x1f, 0x27, 0x26, 0x2e, 0x2f, 0x37, 0x3f, 0x47, 0x46,
          0x36, 0xff, 0x51, 0x03, 0x01, 0x02, 0x64, 0x6c, 0x74, 0x76, 0xff, 0xff},
         {0x09, 0x08, 0x10, 0x18, 0x20, 0x21, 0x29, 0x28, 0x30, 0x38, 0x40, 0x41,
          0x31, 0xff, 0x52, 0x5e, 0x5c, 0x58, 0x60, 0x68, 0x70, 0x6e, 0xff, 0xff},
         {0x11, 0x0a, 0x12, 0x1a, 0x22, 0x23, 0x2b, 0x2a, 0x32, 0x3a, 0x42, 0x43,
          0xff, 0xff, 0x54, 0xff, 0xff, 0xff, 0x61, 0x69, 0x71, 0xff, 0xff, 0xff},
         {0x49, 0xff, 0x0c, 0x14, 0x1c, 0x24, 0x25, 0x2d, 0x2c, 0x34, 0x3c, 0x45,
          0xff, 0xff, 0x4a, 0xff, 0x50, 0xff, 0x62, 0x6a, 0x72, 0x6f, 0xff, 0xff},
         {0x06, 0x5a, 0x4b, 0xff, 0xff, 0xff, 0x5b, 0xff, 0xff, 0xff, 0x4d, 0x4e,
          0x3d, 0xff, 0x04, 0x5f, 0x5d, 0x05, 0x6b, 0xff, 0x73, 0xff, 0xff, 0xff},
         {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
          0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}},
        // Layout size M
        // TODO: Find the key layout matrix of an ANSI M keyboard
        {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
          0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
         {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
          0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
         {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
          0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
         {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
          0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
         {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
          0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
         {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
          0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
         {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
          0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}},
        // Layout size S
        {{0x60, 0x61, 0x62, 0x63, 0x68, 0xFF, 0x69, 0x6A, 0x70, 0x71, 0xFF, 0x72,
          0x43, 0x44, 0x45, 0x66, 0x67, 0x6B, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
         {0x00, 0x01, 0x08, 0x09, 0x10, 0x11, 0x18, 0x19, 0x20, 0x21, 0x28, 0x29,
          0x30, 0xFF, 0x31, 0x38, 0x39, 0x40, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
         {0x02, 0x03, 0x0A, 0x0B, 0x12, 0x13, 0x1A, 0x1B, 0x22, 0x23, 0x2A, 0x2B,
          0x32, 0xFF, 0x33, 0x3A, 0x3B, 0x42, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
         {0x04, 0x05, 0x0C, 0x0D, 0x14, 0x15, 0x1C, 0x1D, 0x24, 0x25, 0x2C, 0x2D,
          0xFF, 0xFF, 0x34, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
         {0x06, 0xFF, 0x07, 0x0E, 0x0F, 0x16, 0x17, 0x1E, 0x1F, 0x26, 0x27, 0x2E,
          0x01, 0xFF, 0x2F, 0xFF, 0x3D, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
         {0x5B, 0x5A, 0x5C, 0xFF, 0xFF, 0xFF, 0x5D, 0xFF, 0xFF, 0xFF, 0x5E, 0x3C,
          0x5F, 0xFF, 0x36, 0x3F, 0x3E, 0x46, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
         {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
          0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}},
    }, { // ISO Layouts
        // Layout size L
        // This key layout matrix was copied from libcmmk, GNU GPLv3
        {{0x0B, 0x16, 0x1E, 0x19, 0x1B, 0xFF, 0x07, 0x33, 0x39, 0x3E, 0xFF, 0x56,
          0x57, 0x53, 0x55, 0x4F, 0x48, 0x00, 0x65, 0x6D, 0x75, 0x77, 0xFF, 0xFF},
         {0x0E, 0x0F, 0x17, 0x1F, 0x27, 0x26, 0x2E, 0x2F, 0x37, 0x3F, 0x47, 0x46,
          0x36, 0xFF, 0x51, 0x03, 0x01, 0x02, 0x64, 0x6C, 0x74, 0x76, 0xFF, 0xFF},
         {0x09, 0x08, 0x10, 0x18, 0x20, 0x21, 0x29, 0x28, 0x30, 0x38, 0x40, 0x41,
          0x31, 0xFF, 0x54, 0x5E, 0x5C, 0x58, 0x60, 0x68, 0x70, 0x6E, 0xFF, 0xFF},
         {0x11, 0x0A, 0x12, 0x1A, 0x22, 0x23, 0x2B, 0x2A, 0x32, 0x3A, 0x42, 0x43,
          0x44, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x61, 0x69, 0x71, 0xFF, 0xFF, 0xFF},
         {0x49, 0x13, 0x0C, 0x14, 0x1C, 0x24, 0x25, 0x2D, 0x2C, 0x34, 0x3C, 0x45,
          0xFF, 0xFF, 0x4A, 0xFF, 0x50, 0xFF, 0x62, 0x6A, 0x72, 0x6F, 0xFF, 0xFF},
         {0x06, 0x5A, 0x4B, 0xFF, 0xFF, 0xFF, 0x5B, 0xFF, 0xFF, 0xFF, 0x4D, 0x4E,
          0x3D, 0xFF, 0x04, 0x5F, 0x5D, 0x05, 0x6B, 0xFF, 0x73, 0xFF, 0xFF, 0xFF},
         {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
          0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}},
        // Layout size M
        // TODO: Find the key layout matrix of an ISO M keyboard
        {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
          0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
         {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
          0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
         {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
          0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
         {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
          0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
         {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
          0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
         {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
          0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
         {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
          0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}},
        // Layout size S
        // TODO: Find the key layout matrix of an ISO S keyboard
        {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
          0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
         {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
          0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
         {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
          0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
         {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
          0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
         {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
          0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
         {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
          0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
         {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
          0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}}
    },
    // TODO: If required, Japanese layouts may be added at index 2 of this array
    // TODO: so that it remains compatible with the LibMK_Layout enum and the
    // TODO: firmware defined layout
};


bool libmk_init(void) {
    int result = libusb_init(&Context);
#ifdef LIBMK_USB_DEBUG
    libusb_set_debug(Context, LIBUSB_LOG_LEVEL_DEBUG);
#endif
    return (result == LIBUSB_SUCCESS);
}


int libmk_exit(void) {
    if (DeviceHandle != NULL) {
        int r = libmk_free_handle(DeviceHandle);
        if (r != LIBMK_SUCCESS)
            return r;
    }
    libusb_exit(Context);
    return LIBMK_SUCCESS;
}


int libmk_detect_devices(LibMK_Model** model_list) {
    libusb_device** devices = NULL;
    ssize_t amount = libusb_get_device_list(Context, &devices);
    if (amount < 0)
        return LIBMK_ERR_DEV_LIST;

    int n = 0;
    libusb_device* current = NULL;
    LibMK_Device* list = NULL;
    LibMK_Device* temp = NULL;

    // Build a linked list of Device structs of supported devices
    for (ssize_t i = 0; i < amount; i++) {
        current = devices[i];
        if (current == NULL)
            break;
        temp = libmk_open_device(current);
        if (temp == NULL)  // Not a MasterKeys device
            continue;

        list = libmk_append_device(list, temp);
        n = n + 1;
    }

    // Now build an array of Model numbers
    LibMK_Model* models = (LibMK_Model*) malloc(sizeof(LibMK_Model) * n);
    *model_list = models;
    temp = list;
    LibMK_Device* to_free;
    for (int j = 0; j < n; j++) {
        models[j] = temp->model;
        to_free = temp;
        if (temp->next != NULL)
            temp = temp->next;
        libmk_free_device(to_free);
    }
    return n;
}


LibMK_Device* libmk_open_device(libusb_device* device) {
    int r;
    struct libusb_device_descriptor descriptor;
    libusb_device_handle* handle = NULL;
    unsigned char* manufacturer =
        (unsigned char*) malloc(LIBMK_USB_DESCR_LEN * sizeof(unsigned char));
    unsigned char* product =
        (unsigned char*) malloc(LIBMK_USB_DESCR_LEN * sizeof(unsigned char));

    // Get the device descriptor of the next device
    r = libusb_get_device_descriptor(device, &descriptor);
    if (r < 0)
        return NULL;

    // Open the device and decode the descriptor strings
    r = libusb_open(device, &handle);
    if (r < 0)
        return NULL;

    libusb_get_string_descriptor_ascii(
        handle, descriptor.iManufacturer,
        manufacturer, LIBMK_USB_DESCR_LEN);
    libusb_get_string_descriptor_ascii(
        handle, descriptor.iProduct,
        product, LIBMK_USB_DESCR_LEN);

    if (strcmp((char*) manufacturer, MANUFACTURER) != 0)
        return NULL;

    // Build a Device descriptor
    LibMK_Model model = libmk_ident_model((char*) product);
    if (model == DEV_UNKNOWN)
        return NULL;

    LibMK_Device* device_struct = libmk_create_device(
        model, device, (char*) manufacturer, (char*) product,
        descriptor.idVendor, descriptor.idProduct);
    libusb_close(handle);
    return device_struct;
}


LibMK_Device* libmk_create_device(LibMK_Model model, libusb_device* dev,
                                  char* iManufacturer, char* iProduct,
                                  int bVendor, int bDevice) {
    char* m_str = (char*) malloc((strlen(iManufacturer) + 1) * sizeof(char));
    char* p_str = (char*) malloc((strlen(iProduct) + 1) * sizeof(char));
    strcpy(m_str, iManufacturer);
    strcpy(p_str, iProduct);
    LibMK_Device* device = (LibMK_Device*) malloc(sizeof(LibMK_Device));
    device->iManufacturer = m_str;
    device->iProduct = p_str;
    device->bVendor = bVendor;
    device->bDevice = bDevice;
    device->device = dev;
    device->model = model;
    return device;
}


void libmk_free_device(LibMK_Device* device) {
    free(device->iManufacturer);
    free(device->iProduct);
    free(device);
}


LibMK_Device* libmk_append_device(LibMK_Device* first_device,
                                  LibMK_Device* device) {
    if (first_device == NULL)
        return device;
    LibMK_Device* current = first_device;
    while (current->next != NULL)
        current = current->next;
    current->next = device;
    return first_device;
}


int libmk_create_handle(LibMK_Handle** handle, LibMK_Device* device) {
    *handle = (LibMK_Handle*) malloc(sizeof(LibMK_Handle));
    if (*handle == NULL)
        return LIBMK_ERR_DEV_OPEN_FAILED;
    int r = libusb_open(device->device, &(*handle)->handle);
    if (r != 0)
        return LIBMK_ERR_DEV_OPEN_FAILED;
    (*handle)->open = true;
    (*handle)->model = device->model;
    (*handle)->bDevice = device->bDevice;
    (*handle)->bVendor = device->bVendor;
    LibMK_Model model = device->model;
    if (model == DEV_RGB_L || model == DEV_WHITE_L)
        (*handle)->size = LIBMK_L;
    else if (model == DEV_RGB_M || model == DEV_WHITE_M)
        (*handle)->size = LIBMK_M;
    else if (model == DEV_RGB_S || model == DEV_WHITE_S)
        (*handle)->size = LIBMK_S;
    else {
        libusb_close((*handle)->handle);
        return LIBMK_ERR_UNKNOWN_LAYOUT;
    }
    return LIBMK_SUCCESS;
}


int libmk_free_handle(LibMK_Handle* handle) {
    if (handle->open)
        return LIBMK_ERR_DEV_NOT_CLOSED;
    free(handle);
    return LIBMK_SUCCESS;
}


int libmk_set_device(LibMK_Model model, LibMK_Handle** handle) {
    libusb_device** devices;
    ssize_t amount = libusb_get_device_list(NULL, &devices);
    if (amount < 0)
        return LIBMK_ERR_DEV_LIST;

    // Variables required for looping over the device list
    int r;  // libusb requires error handling with return values
    libusb_device* current = NULL;
    libusb_device* device = NULL;
    struct libusb_device_descriptor descriptor;
    libusb_device_handle* usb_handle;

    // Strings to store decoded device descriptors in
    unsigned char manufacturer[LIBMK_USB_DESCR_LEN];
    unsigned char product[LIBMK_USB_DESCR_LEN];

    for (ssize_t i = 0; i < amount; i++) {

        // Get the device descriptor of the next device
        current = devices[i];
        r = libusb_get_device_descriptor(current, &descriptor);
        if (r < 0)
            return LIBMK_ERR_DESCR;

        // Open the device and decode the descriptor strings
        r = libusb_open(current, &usb_handle);
        if (r < 0)
            return LIBMK_ERR_DEV_OPEN_FAILED;
        libusb_get_string_descriptor_ascii(
            usb_handle, descriptor.iManufacturer,
            manufacturer, LIBMK_USB_DESCR_LEN);
        libusb_get_string_descriptor_ascii(
            usb_handle, descriptor.iProduct,
            product, LIBMK_USB_DESCR_LEN);

        // Determine whether this is a valid MasterKeys keyboard device
        if (strcmp((char*) manufacturer, MANUFACTURER) == 0) {
            if (model == DEV_ANY
                || libmk_ident_model((char*) product) == model) {

                device = current;
                libusb_close(usb_handle);
                break;
            }
        }
        libusb_close(usb_handle);
    }

    if (device == NULL)
        // No devices detected
        return LIBMK_ERR_DEV_NOT_CONNECTED;

    // Open the device into the handle
    LibMK_Device* dev = libmk_open_device(device);
    if (handle == NULL)
        handle = &DeviceHandle;
    r = libmk_create_handle(handle, dev);
    libusb_free_device_list(devices, true);
    return r;
}


LibMK_Model libmk_ident_model(char* product) {
    if (strstr(product, PRODUCT) == NULL)
        return DEV_UNKNOWN;
    if (strstr(product, WHITE) != NULL) {
        if (strstr(product, LARGE) != NULL)
            return DEV_WHITE_L;
        else if (strstr(product, SMALL) != NULL)
            return DEV_WHITE_S;
        return DEV_WHITE_M;
    }
    if (strstr(product, LARGE) != NULL)
        return DEV_RGB_L;
    else if (strstr(product, SMALL) != NULL)
        return DEV_RGB_S;
    return DEV_RGB_M;
}


int libmk_get_device_ident(LibMK_Handle* handle) {
    if (handle == NULL)
        handle = DeviceHandle;
    if (handle == NULL)
        return 0x0000;
    return DeviceHandle->bDevice;
}


int libmk_enable_control(LibMK_Handle* handle) {
    int r;  // Stores libusb return values

    if (handle == NULL)
        handle = DeviceHandle;
    if (handle == NULL)
        return LIBMK_ERR_DEV_NOT_SET;

    // Claim the interface on the device
    r = libmk_claim_interface(handle);
    if (r != LIBMK_SUCCESS)
        return LIBMK_ERR_IFACE_CLAIM_FAILED;

    // Send the enable control packet to the keyboard
    return libmk_send_control_packet(handle);
}


int libmk_send_control_packet(LibMK_Handle* handle) {
    int r = libmk_set_control_mode(handle, LIBMK_CUSTOM_CTRL);
    if (r != LIBMK_SUCCESS)
        return r;
    LibMK_Firmware* fw;
    r = libmk_get_firmware_version(handle, &fw);
    if (r != LIBMK_SUCCESS) {
        libusb_close(handle->handle);
        return r;
    }
    handle->layout = fw->layout;
    return LIBMK_SUCCESS;
}


int libmk_disable_control(LibMK_Handle* handle) {
    if (handle == NULL)
        handle = DeviceHandle;
    if (handle == NULL)
        return LIBMK_ERR_DEV_NOT_SET;

    int r = libmk_set_control_mode(handle, LIBMK_FIRMWARE_CTRL);
    if (r != LIBMK_SUCCESS)
        return r;

    r = libusb_release_interface(handle->handle, LIBMK_IFACE_NUM);
    if (r < 0)
        return LIBMK_ERR_IFACE_RELEASE_FAILED;

    libusb_close(handle->handle);
    handle->open = false;
    if (handle == DeviceHandle) {
        libmk_free_handle(handle);
        DeviceHandle = NULL;
    }
    libusb_reset_device(handle->handle);
    return LIBMK_SUCCESS;
}


int libmk_claim_interface(LibMK_Handle* handle) {
    int r;
    if (handle == NULL)
        handle = DeviceHandle;
    if (handle == NULL)
        return LIBMK_ERR_DEV_NOT_SET;

    // Unload the kernel driver for the device if it is active
    if (libusb_kernel_driver_active(handle->handle, LIBMK_IFACE_NUM)) {
        r = libusb_detach_kernel_driver(handle->handle, LIBMK_IFACE_NUM);
        if (r < 0)
            return LIBMK_ERR_KERNEL_DRIVER;
    }

    // Claim the interface on the device
    r = libusb_claim_interface(handle->handle, LIBMK_IFACE_NUM);
    if (r < 0)
        return LIBMK_ERR_IFACE_CLAIM_FAILED;
    return LIBMK_SUCCESS;
}


int libmk_set_effect(LibMK_Handle* handle, LibMK_Effect effect) {
    if (handle == NULL)
        handle = DeviceHandle;
    if (handle == NULL)
        return LIBMK_ERR_DEV_NOT_SET;

    unsigned char* packet = libmk_build_packet(
        2, 0x41, 0x01);
    int r = libmk_send_packet(handle, packet);
    if (r != LIBMK_SUCCESS)
        return r;
    packet = libmk_build_packet(
        5, HEADER_SET, OPCODE_EFFECT, 0x00, 0x00, (unsigned char) effect);
    return libmk_send_packet(handle, packet);
}


int libmk_set_full_color(LibMK_Handle* handle,
                         unsigned char r,
                         unsigned char g,
                         unsigned char b) {
    if (handle == NULL)
        handle = DeviceHandle;
    if (handle == NULL)
        return LIBMK_ERR_DEV_NOT_SET;
    unsigned char* packet = libmk_build_packet(
        7, HEADER_FULL_COLOR, 0x00, 0x00, 0x00, r, g, b);
    return libmk_send_packet(handle, packet);
}


int libmk_send_packet(LibMK_Handle* handle, unsigned char* packet) {
    return libmk_send_recv_packet(handle, packet, true);
}


int libmk_send_recv_packet(
        LibMK_Handle* handle, unsigned char* packet, bool response_required) {
    if (handle == NULL)
        handle = DeviceHandle;
    if (handle == NULL)
        return LIBMK_ERR_DEV_NOT_SET;
    int t, result;
    int r = libusb_interrupt_transfer(
        handle->handle, LIBMK_EP_OUT | LIBUSB_ENDPOINT_OUT,
        packet, LIBMK_PACKET_SIZE, &t,
        LIBMK_PACKET_TIMEOUT);
#ifdef LIBMK_DEBUG
    libmk_print_packet(packet, "Sent");
#endif // LIBMK_DEBUG
    free(packet);
    if (r != 0 || t != LIBMK_PACKET_SIZE)
        return LIBMK_ERR_TRANSFER;
    packet = libmk_build_packet(0);
    r = libusb_interrupt_transfer(
            handle->handle, LIBMK_EP_IN | LIBUSB_ENDPOINT_IN,
            packet, LIBMK_PACKET_SIZE, &t, LIBMK_PACKET_TIMEOUT);
#ifdef LIBMK_DEBUG
    libmk_print_packet(packet, "Response");
#endif // LIBMK_DEBUG
    if (r != LIBUSB_SUCCESS && response_required) {
        result = LIBMK_ERR_TRANSFER;
    } else if (t != LIBMK_PACKET_SIZE && response_required) {
        result = LIBMK_ERR_TRANSFER;
    } else if (packet[0] == HEADER_ERROR) {
        libmk_print_packet(packet, "Error response");
        result = LIBMK_ERR_PROTOCOL;
    } else
        result = LIBMK_SUCCESS;
    free(packet);
    return result;
}


int libmk_exch_packet(LibMK_Handle* handle, unsigned char* packet) {
    int t;
    int r = libusb_interrupt_transfer(
        handle->handle, LIBMK_EP_OUT | LIBUSB_ENDPOINT_OUT,
        packet, LIBMK_PACKET_SIZE, &t, LIBMK_PACKET_TIMEOUT);
#ifdef LIBMK_DEBUG
    libmk_print_packet(packet, "Sent");
#endif // LIBMK_DEBUG
    if (r != LIBUSB_SUCCESS || t != LIBMK_PACKET_SIZE) {
        free(packet);
        return LIBMK_ERR_TRANSFER;
    }
    r = libusb_interrupt_transfer(
        handle->handle, LIBMK_EP_IN | LIBUSB_ENDPOINT_IN,
        packet, LIBMK_PACKET_SIZE, &t, LIBMK_PACKET_TIMEOUT);
#ifdef LIBMK_DEBUG
    libmk_print_packet(packet, "Received");
#endif // LIBMK_DEBUG
    if (r != LIBUSB_SUCCESS || t != LIBMK_PACKET_SIZE)
        return LIBMK_ERR_TRANSFER;
    return LIBMK_SUCCESS;
}


unsigned char* libmk_build_packet(unsigned char predef, ...) {
    unsigned char* packet = (unsigned char*)
        malloc(LIBMK_PACKET_SIZE * sizeof(unsigned char));
    if (packet == NULL)
        return NULL;
    for (int i = 0; i < LIBMK_PACKET_SIZE; i++)
        packet[i] = 0x00;
    va_list elements;
    va_start(elements, predef);
    int elem;
    for (unsigned char i = 0; i < predef; i++) {
        elem = va_arg(elements, int);
        packet[i] = (unsigned char) elem;
    }
    va_end(elements);
    return packet;
}


int libmk_reset(LibMK_Handle* handle) {
    if (handle == NULL)
        handle = DeviceHandle;
    if (handle == NULL)
        return LIBMK_ERR_DEV_NOT_SET;
    int r = libusb_reset_device(handle->handle);
    if (r != LIBUSB_SUCCESS)
        return LIBMK_ERR_DEV_RESET_FAILED;
    return LIBMK_SUCCESS;
}


int libmk_set_all_led_color(LibMK_Handle* handle, unsigned char* colors) {
    if (handle == NULL)
        handle = DeviceHandle;
    if (handle == NULL)
        return LIBMK_ERR_DEV_NOT_SET;
    libmk_set_effect(handle, LIBMK_EFF_CUSTOM);
    unsigned char* packets[LIBMK_ALL_LED_PCK_NUM];

    for (short i = 0; i < LIBMK_ALL_LED_PCK_NUM; i++)
        packets[i] = libmk_build_packet(3, HEADER_SET, 0xA8, (unsigned char) i * 2);

    unsigned char offset;
    int packet, index, result;

    for (unsigned char r = 0; r < LIBMK_MAX_ROWS; r++)
        for (unsigned char c = 0; c < LIBMK_MAX_COLS; c++) {
            result = libmk_get_offset(&offset, handle, r, c);
            if (result != LIBMK_SUCCESS)
                return result;
            if (offset == 0xFF)
                continue;
            packet = offset / LIBMK_ALL_LED_PER_PCK;
            index = offset % LIBMK_ALL_LED_PER_PCK;
            for (short o = 0; o < 3; o++) {
                packets[packet][4 + (index * 3) + o] = colors[
                    (r * LIBMK_MAX_COLS + c) * 3 + o];
            }
        }
        
    for (short k = 0; k < LIBMK_ALL_LED_PCK_NUM; k++) {
        int r = libmk_send_packet(handle, packets[k]);
        if (r != LIBMK_SUCCESS)
            return r;
    }
    return LIBMK_SUCCESS;
}


inline void libmk_print_packet(unsigned char* packet, char* label) {
#ifdef LIBMK_DEBUG
    printf("Packet: %s\n", label);
    for (unsigned char j = 0; j < LIBMK_PACKET_SIZE; j++) {
        printf("%02x ", packet[j]);
        if ((j + 1) % 16 == 0) {
            printf("\n");
        } else if ((j + 1) % 8 == 0)
            printf(" ");
    }
#endif
}


int libmk_get_offset(
        unsigned char* offset, LibMK_Handle* handle,
        unsigned char row, unsigned char col) {
    if (handle->layout != LAYOUT_ISO && handle->layout != LAYOUT_ANSI)
        return LIBMK_ERR_UNKNOWN_LAYOUT;
    *offset = LIBMK_LAYOUT[handle->layout - 1][handle->size][row][col];
    return LIBMK_SUCCESS;
}


int libmk_set_single_led(
        LibMK_Handle* handle, unsigned char row, unsigned char col,
        unsigned char r, unsigned char g, unsigned char b) {
    if (handle == NULL)
        handle = DeviceHandle;
    if (handle == NULL)
        return LIBMK_ERR_DEV_NOT_SET;
    int result = libmk_set_control_mode(handle, LIBMK_CUSTOM_CTRL);
    if (result != LIBMK_SUCCESS)
        return result;
    unsigned char offset;
    result = libmk_get_offset(&offset, handle, row, col);
    if (result != LIBMK_SUCCESS)
        return result;
    unsigned char* packet = libmk_build_packet(
        8, 0xC0, 0x01, 0x01, 0x00, offset, r, g, b);
    return libmk_send_packet(handle, packet);
}


int libmk_set_effect_details(
    LibMK_Handle* handle, LibMK_Effect_Details* effect) {
    if (handle == NULL)
        handle = DeviceHandle;
    if (handle == NULL)
        return LIBMK_ERR_DEV_NOT_SET;
    unsigned char* packet = libmk_build_packet(
        10, HEADER_SET, OPCODE_EFFECT_ARGS, 0x00, 0x00,
        (unsigned char) effect->effect, effect->speed, effect->direction,
        effect->amount, 0xFF, 0xFF);
    unsigned char i;
    for (i=0; i < 3; i++)
        packet[10 + i] = effect->foreground[i];
    for (i=0; i < 3; i++)
        packet[13 + i] = effect->background[i];
    for (i=16; i < 64; i++)
        packet[i] = 0xFF;
    int r = libmk_set_effect(handle, effect->effect);
    if (r != LIBMK_SUCCESS)
        return r;
    return libmk_send_packet(handle, packet);
}


int libmk_get_firmware_version(LibMK_Handle* handle, LibMK_Firmware** fw) {
    if (handle == NULL)
        handle = DeviceHandle;
    if (handle == NULL)
        return LIBMK_ERR_DEV_NOT_SET;
    unsigned char* p = libmk_build_packet(2, 0x01, 0x02);
    int r = libmk_exch_packet(handle, p);
    if (r != LIBMK_SUCCESS)
        return r;
    (*fw) = (LibMK_Firmware*) malloc(sizeof(LibMK_Firmware));
    for (unsigned char i=0; i<5; i++)
        (*fw)->string[i] = p[0x04 + i];
    (*fw)->string[5] = 0x00;
    (*fw)->major = p[0x04] & 0x0F;
    (*fw)->minor = p[0x06] & 0x0F;
    (*fw)->patch = p[0x08] & 0x0F;
    (*fw)->layout = p[0x04] & 0x0F;
    return LIBMK_SUCCESS;
}


int libmk_save_profile(LibMK_Handle* handle) {
     if (handle == NULL)
         handle = DeviceHandle;
     if (handle == NULL)
         return LIBMK_ERR_DEV_NOT_SET;
     int r = libmk_set_control_mode(handle, LIBMK_PROFILE_CTRL);
     if (r != LIBMK_SUCCESS)
         return r;
     unsigned char* p = libmk_build_packet(2, 0x50, 0x55);
     r = libmk_send_recv_packet(handle, p, false);
     if (r != LIBMK_SUCCESS)
         return r;
     return libmk_set_control_mode(handle, LIBMK_CUSTOM_CTRL);
}


int libmk_set_active_profile(LibMK_Handle* handle, char profile) {
    if (handle == NULL)
        handle = DeviceHandle;
    if (handle == NULL)
        return LIBMK_ERR_DEV_NOT_SET;
    int r = libmk_set_control_mode(handle, LIBMK_PROFILE_CTRL);
    if (r != LIBMK_SUCCESS)
        return r;
    if (!(1 <= profile <= 4))
        return LIBMK_ERR_INVALID_ARG;
    unsigned char* p = libmk_build_packet(5, HEADER_SET, 0x00, 0x00, 0x00, profile);
    r = libmk_send_packet(handle, p);
    if (r != LIBMK_SUCCESS)
        return r;
    return libmk_set_control_mode(handle, LIBMK_CUSTOM_CTRL);
}


int libmk_get_active_profile(LibMK_Handle* handle, char* profile) {
    if (handle == NULL)
        handle = DeviceHandle;
    if (handle == NULL)
        return LIBMK_ERR_DEV_NOT_SET;
    unsigned char* p = libmk_build_packet(1, 0x52);
    int r = libmk_exch_packet(handle, p);
    if (r != LIBMK_SUCCESS)
        return r;
    *profile = p[4];
    free(p);
    return LIBMK_SUCCESS;
}


int libmk_set_control_mode(LibMK_Handle* handle, LibMK_ControlMode mode) {
    if (handle == NULL)
        handle = DeviceHandle;
    if (handle == NULL)
        return LIBMK_ERR_DEV_NOT_SET;
    char* p = libmk_build_packet(2, 0x41, mode);
    return libmk_send_recv_packet(handle, p, false);
}
